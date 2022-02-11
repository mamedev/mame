// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
/*
    Intel 386 emulator

    Written by Ville Linde

    Currently supports:
        Intel 386
        Intel 486
        Intel Pentium
        Cyrix MediaGX
        Intel Pentium MMX
        Intel Pentium Pro
        Intel Pentium II
        Intel Pentium III
        Amd Athlon XP (athlon.cpp)
        Intel Pentium 4
*/

#include "emu.h"
#include "i386.h"
#include "i386priv.h"
#include "x87priv.h"
#include "cycles.h"
#include "i386ops.h"

#include "debugger.h"
#include "debug/debugcpu.h"
#include "debug/express.h"

/* seems to be defined on mingw-gcc */
#undef i386

DEFINE_DEVICE_TYPE(I386,        i386_device,        "i386",        "Intel I386")
DEFINE_DEVICE_TYPE(I386SX,      i386sx_device,      "i386sx",      "Intel I386SX")
DEFINE_DEVICE_TYPE(I486,        i486_device,        "i486",        "Intel I486")
DEFINE_DEVICE_TYPE(I486DX4,     i486dx4_device,     "i486dx4",     "Intel I486DX4")
DEFINE_DEVICE_TYPE(PENTIUM,     pentium_device,     "pentium",     "Intel Pentium")
DEFINE_DEVICE_TYPE(PENTIUM_MMX, pentium_mmx_device, "pentium_mmx", "Intel Pentium MMX")
DEFINE_DEVICE_TYPE(MEDIAGX,     mediagx_device,     "mediagx",     "Cyrix MediaGX")
DEFINE_DEVICE_TYPE(PENTIUM_PRO, pentium_pro_device, "pentium_pro", "Intel Pentium Pro")
DEFINE_DEVICE_TYPE(PENTIUM2,    pentium2_device,    "pentium2",    "Intel Pentium II")
DEFINE_DEVICE_TYPE(PENTIUM3,    pentium3_device,    "pentium3",    "Intel Pentium III")
DEFINE_DEVICE_TYPE(PENTIUM4,    pentium4_device,    "pentium4",    "Intel Pentium 4")


i386_device::i386_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i386_device(mconfig, I386, tag, owner, clock, 32, 32, 32)
{
}


i386_device::i386_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_data_width, int program_addr_width, int io_data_width)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_vtlb_interface(mconfig, *this, AS_PROGRAM)
	, m_program_config("program", ENDIANNESS_LITTLE, program_data_width, program_addr_width, 0, 32, 12)
	, m_io_config("io", ENDIANNESS_LITTLE, io_data_width, 16, 0)
	, m_smiact(*this)
	, m_ferr_handler(*this)
{
	// 32 unified
	set_vtlb_dynamic_entries(32);
}

i386sx_device::i386sx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i386_device(mconfig, I386SX, tag, owner, clock, 16, 24, 16)
{
}

i486_device::i486_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i486_device(mconfig, I486, tag, owner, clock)
{
}

i486_device::i486_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: i386_device(mconfig, type, tag, owner, clock, 32, 32, 32)
{
}

i486dx4_device::i486dx4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i486_device(mconfig, I486DX4, tag, owner, clock)
{
}

pentium_device::pentium_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_device(mconfig, PENTIUM, tag, owner, clock)
{
}

pentium_device::pentium_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: i386_device(mconfig, type, tag, owner, clock, 32, 32, 32)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb
	set_vtlb_dynamic_entries(96);
}

mediagx_device::mediagx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i386_device(mconfig, MEDIAGX, tag, owner, clock, 32, 32, 32)
{
}

pentium_pro_device::pentium_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_pro_device(mconfig, PENTIUM_PRO, tag, owner, clock)
{
}

pentium_pro_device::pentium_pro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pentium_device(mconfig, type, tag, owner, clock)
{
}

pentium_mmx_device::pentium_mmx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_device(mconfig, PENTIUM_MMX, tag, owner, clock)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	set_vtlb_dynamic_entries(96);
}

pentium2_device::pentium2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_pro_device(mconfig, PENTIUM2, tag, owner, clock)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	set_vtlb_dynamic_entries(96);
}

pentium3_device::pentium3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_pro_device(mconfig, PENTIUM3, tag, owner, clock)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	set_vtlb_dynamic_entries(96);
}

pentium4_device::pentium4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pentium_device(mconfig, PENTIUM4, tag, owner, clock)
{
	// 128 dtlb, 64 itlb
	set_vtlb_dynamic_entries(196);
}

device_memory_interface::space_config_vector i386_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

int i386_parity_table[256];
MODRM_TABLE i386_MODRM_table[256];

/*************************************************************************/

uint32_t i386_device::i386_translate(int segment, uint32_t ip, int rwn)
{
	// TODO: segment limit access size, execution permission, handle exception thrown from exception handler
	if (PROTECTED_MODE && !V8086_MODE && (rwn != -1))
	{
		if (!(m_sreg[segment].valid))
			FAULT_THROW((segment == SS) ? FAULT_SS : FAULT_GP, 0);
		if (i386_limit_check(segment, ip))
			FAULT_THROW((segment == SS) ? FAULT_SS : FAULT_GP, 0);
		if ((rwn == 0) && ((m_sreg[segment].flags & 8) && !(m_sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
		if ((rwn == 1) && ((m_sreg[segment].flags & 8) || !(m_sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
	}
	return m_sreg[segment].base + ip;
}

vtlb_entry i386_device::get_permissions(uint32_t pte, int wp)
{
	vtlb_entry ret = VTLB_READ_ALLOWED | ((pte & 4) ? VTLB_USER_READ_ALLOWED : 0);
	if (!wp)
		ret |= VTLB_WRITE_ALLOWED;
	if (pte & 2)
		ret |= VTLB_WRITE_ALLOWED | ((pte & 4) ? VTLB_USER_WRITE_ALLOWED : 0);
	return ret;
}

bool i386_device::i386_translate_address(int intention, offs_t *address, vtlb_entry *entry)
{
	uint32_t a = *address;
	uint32_t pdbr = m_cr[3] & 0xfffff000;
	uint32_t directory = (a >> 22) & 0x3ff;
	uint32_t table = (a >> 12) & 0x3ff;
	vtlb_entry perm = 0;
	bool ret;
	bool user = (intention & TRANSLATE_USER_MASK) ? true : false;
	bool write = (intention & TRANSLATE_WRITE) ? true : false;
	bool debug = (intention & TRANSLATE_DEBUG_MASK) ? true : false;

	if (!(m_cr[0] & 0x80000000))
	{
		if (entry)
			*entry = 0x77;
		return true;
	}

	uint32_t page_dir = m_program->read_dword(pdbr + directory * 4);
	if (page_dir & 1)
	{
		if ((page_dir & 0x80) && (m_cr[4] & 0x10))
		{
			a = (page_dir & 0xffc00000) | (a & 0x003fffff);
			if (debug)
			{
				*address = a;
				return true;
			}
			perm = get_permissions(page_dir, WP);
			if (write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
				ret = false;
			else if (user && !(perm & VTLB_USER_READ_ALLOWED))
				ret = false;
			else
			{
				if (write)
					perm |= VTLB_FLAG_DIRTY;
				if (!(page_dir & 0x40) && write)
					m_program->write_dword(pdbr + directory * 4, page_dir | 0x60);
				else if (!(page_dir & 0x20))
					m_program->write_dword(pdbr + directory * 4, page_dir | 0x20);
				ret = true;
			}
		}
		else
		{
			uint32_t page_entry = m_program->read_dword((page_dir & 0xfffff000) + (table * 4));
			if (!(page_entry & 1))
				ret = false;
			else
			{
				a = (page_entry & 0xfffff000) | (a & 0xfff);
				if (debug)
				{
					*address = a;
					return true;
				}
				perm = get_permissions(page_entry, WP);
				if (write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
					ret = false;
				else if (user && !(perm & VTLB_USER_READ_ALLOWED))
					ret = false;
				else
				{
					if (write)
						perm |= VTLB_FLAG_DIRTY;
					if (!(page_dir & 0x20))
						m_program->write_dword(pdbr + directory * 4, page_dir | 0x20);
					if (!(page_entry & 0x40) && write)
						m_program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x60);
					else if (!(page_entry & 0x20))
						m_program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x20);
					ret = true;
				}
			}
		}
	}
	else
		ret = false;
	if (entry)
		*entry = perm;
	if (ret)
		*address = a;
	return ret;
}

//#define TEST_TLB

bool i386_device::translate_address(int pl, int type, uint32_t *address, uint32_t *error)
{
	if (!(m_cr[0] & 0x80000000)) // Some (very few) old OS's won't work with this
		return true;

	const vtlb_entry *table = vtlb_table();
	uint32_t index = *address >> 12;
	vtlb_entry entry = table[index];
	if (type == TRANSLATE_FETCH)
		type = TRANSLATE_READ;
	if (pl == 3)
		type |= TRANSLATE_USER_MASK;
#ifdef TEST_TLB
	uint32_t test_addr = *address;
#endif

	if (!(entry & VTLB_FLAG_VALID) || ((type & TRANSLATE_WRITE) && !(entry & VTLB_FLAG_DIRTY)))
	{
		if (!i386_translate_address(type, address, &entry))
		{
			*error = ((type & TRANSLATE_WRITE) ? 2 : 0) | ((m_CPL == 3) ? 4 : 0);
			if (entry)
				*error |= 1;
			return false;
		}
		vtlb_dynload(index, *address, entry);
		return true;
	}
	if (!(entry & (1 << type)))
	{
		*error = ((type & TRANSLATE_WRITE) ? 2 : 0) | ((m_CPL == 3) ? 4 : 0) | 1;
		return false;
	}
	*address = (entry & 0xfffff000) | (*address & 0xfff);
#ifdef TEST_TLB
	int test_ret = i386_translate_address(type | TRANSLATE_DEBUG_MASK, &test_addr, nullptr);
	if (!test_ret || (test_addr != *address))
		logerror("TLB-PTE mismatch! %06X %06X %06x\n", *address, test_addr, m_pc);
#endif
	return true;
}

/***********************************************************************************/

void i386_device::CHANGE_PC(uint32_t pc)
{
	m_pc = i386_translate(CS, pc, -1 );
}

void i386_device::NEAR_BRANCH(int32_t offs)
{
	/* TODO: limit */
	m_eip += offs;
	m_pc += offs;
}

uint8_t i386_device::FETCH()
{
	uint8_t value;
	uint32_t address = m_pc, error;

	if(!translate_address(m_CPL,TRANSLATE_FETCH,&address,&error))
		PF_THROW(error);

	value = mem_pr8(address & m_a20_mask);
#ifdef DEBUG_MISSING_OPCODE
	m_opcode_bytes[m_opcode_bytes_length] = value;
	m_opcode_bytes_length = (m_opcode_bytes_length + 1) & 15;
#endif
	m_eip++;
	m_pc++;
	return value;
}
uint16_t i386_device::FETCH16()
{
	uint16_t value;
	uint32_t address = m_pc, error;

	if( !WORD_ALIGNED(address) ) {       /* Unaligned read */
		value = (FETCH() << 0);
		value |= (FETCH() << 8);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);
		address &= m_a20_mask;
		value = mem_pr16(address);
		m_eip += 2;
		m_pc += 2;
	}
	return value;
}
uint32_t i386_device::FETCH32()
{
	uint32_t value;
	uint32_t address = m_pc, error;

	if( !DWORD_ALIGNED(m_pc) ) {      /* Unaligned read */
		value = (FETCH() << 0);
		value |= (FETCH() << 8);
		value |= (FETCH() << 16);
		value |= (FETCH() << 24);
	} else {
		if(!translate_address(m_CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = mem_pr32(address);
		m_eip += 4;
		m_pc += 4;
	}
	return value;
}

uint8_t i386_device::READ8PL(uint32_t ea, uint8_t privilege)
{
	uint32_t address = ea, error;

	if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
		PF_THROW(error);

	address &= m_a20_mask;
	return m_program->read_byte(address);
}

uint16_t i386_device::READ16PL(uint32_t ea, uint8_t privilege)
{
	uint16_t value;
	uint32_t address = ea, error;

	switch (ea & 3)
	{
	case 0:
	case 2:
	default:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = m_program->read_word(address);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = m_program->read_dword(address - 1, 0x00ffff00) >> 8;
		break;

	case 3:
		value = READ8PL(ea, privilege);
		value |= READ8PL(ea + 1, privilege) << 8;
		break;
	}

	return value;
}

uint32_t i386_device::READ32PL(uint32_t ea, uint8_t privilege)
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
		value = m_program->read_dword(address);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = m_program->read_dword(address - 1, 0xffffff00) >> 8;
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
		value |= m_program->read_dword(address, 0x00ffffff) << 8;
		break;
	}

	return value;
}

uint64_t i386_device::READ64PL(uint32_t ea, uint8_t privilege)
{
	uint64_t value;
	uint32_t address = ea, error;

	switch (ea & 3)
	{
	case 0:
	default:
		value = READ32PL(ea, privilege);
		value |= uint64_t(READ32PL(ea + 4, privilege)) << 32;
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		value = m_program->read_dword(address - 1, 0xffffff00) >> 8;
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
		value |= uint64_t(m_program->read_dword(address, 0x00ffffff)) << 40;
		break;
	}

	return value;
}

uint16_t i386sx_device::READ16PL(uint32_t ea, uint8_t privilege)
{
	uint16_t value;
	uint32_t address = ea, error;

	if (WORD_ALIGNED(ea))
	{
		if(!translate_address(privilege,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		return m_program->read_word(address);
	}
	else
	{
		/* Unaligned read */
		value = READ8PL(ea, privilege);
		value |= READ8PL(ea + 1, privilege) << 8;
		return value;
	}
}

uint32_t i386sx_device::READ32PL(uint32_t ea, uint8_t privilege)
{
	uint32_t value;

	if (WORD_ALIGNED(ea))
	{
		value = READ16PL(ea, privilege);
		value |= READ16PL(ea + 2, privilege) << 16;
		return value;
	}
	else
	{
		value = READ8PL(ea, privilege);
		value |= READ16PL(ea + 1, privilege) << 8;
		value |= READ8PL(ea + 3, privilege) << 24;
		return value;
	}
}

uint64_t i386sx_device::READ64PL(uint32_t ea, uint8_t privilege)
{
	uint64_t value;

	if (WORD_ALIGNED(ea))
	{
		value = READ16PL(ea, privilege);
		value |= uint64_t(READ16PL(ea + 2, privilege)) << 16;
		value |= uint64_t(READ16PL(ea + 4, privilege)) << 32;
		value |= uint64_t(READ16PL(ea + 6, privilege)) << 48;
		return value;
	}
	else
	{
		value = READ8PL(ea, privilege);
		value |= uint64_t(READ16PL(ea + 1, privilege)) << 8;
		value |= uint64_t(READ16PL(ea + 3, privilege)) << 24;
		value |= uint64_t(READ16PL(ea + 5, privilege)) << 40;
		value |= uint64_t(READ8PL(ea + 7, privilege)) << 56;
		return value;
	}
}

void i386_device::WRITE_TEST(uint32_t ea)
{
	uint32_t address = ea, error;
	if(!translate_address(m_CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);
}

void i386_device::WRITE8PL(uint32_t ea, uint8_t privilege, uint8_t value)
{
	uint32_t address = ea, error;
	if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);

	address &= m_a20_mask;
	m_program->write_byte(address, value);
}

void i386_device::WRITE16PL(uint32_t ea, uint8_t privilege, uint16_t value)
{
	uint32_t address = ea, error;

	switch(ea & 3)
	{
	case 0:
	case 2:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		m_program->write_word(address, value);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		m_program->write_dword(address - 1, value << 8, 0x00ffff00);
		break;

	case 3:
		WRITE8PL(ea, privilege, value & 0xff);
		WRITE8PL(ea + 1, privilege, (value >> 8) & 0xff);
		break;
	}
}

void i386_device::WRITE32PL(uint32_t ea, uint8_t privilege, uint32_t value)
{
	uint32_t address = ea, error;

	switch(ea & 3)
	{
	case 0:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		m_program->write_dword(address, value);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		m_program->write_dword(address - 1, (value << 8) & 0xffffff00, 0xffffff00);
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
		m_program->write_dword(address, value >> 8, 0x00ffffff);
		break;
	}
}

void i386_device::WRITE64PL(uint32_t ea, uint8_t privilege, uint64_t value)
{
	uint32_t address = ea, error;

	switch(ea & 3)
	{
	case 0:
		WRITE32PL(ea, privilege, value & 0xffffffff);
		WRITE32PL(ea + 4, privilege, (value >> 32) & 0xffffffff);
		break;

	case 1:
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		m_program->write_dword(address - 1, value << 8, 0xffffff00);
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
		m_program->write_dword(address, (value >> 40) & 0x00ffffff, 0x00ffffff);
		break;
	}
}

void i386sx_device::WRITE16PL(uint32_t ea, uint8_t privilege, uint16_t value)
{
	uint32_t address = ea, error;

	if (WORD_ALIGNED(ea))
	{
		if(!translate_address(privilege,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= m_a20_mask;
		m_program->write_word(address, value);
	}
	else
	{
		WRITE8PL(ea, privilege, value & 0xff);
		WRITE8PL(ea + 1, privilege, (value >> 8) & 0xff);
	}
}

void i386sx_device::WRITE32PL(uint32_t ea, uint8_t privilege, uint32_t value)
{
	if (WORD_ALIGNED(ea))
	{
		WRITE16PL(ea, privilege, value & 0xffff);
		WRITE16PL(ea + 2, privilege, (value >> 16) & 0xffff);
	}
	else
	{
		WRITE8PL(ea, privilege, value & 0xff);
		WRITE16PL(ea + 1, privilege, (value >> 8) & 0xffff);
		WRITE8PL(ea + 3, privilege, (value >> 24) & 0xff);
	}
}

void i386sx_device::WRITE64PL(uint32_t ea, uint8_t privilege, uint64_t value)
{
	if (WORD_ALIGNED(ea))
	{
		WRITE16PL(ea, privilege, value & 0xffff);
		WRITE16PL(ea + 2, privilege, (value >> 16) & 0xffff);
		WRITE16PL(ea + 4, privilege, (value >> 32) & 0xffff);
		WRITE16PL(ea + 6, privilege, (value >> 48) & 0xffff);
	}
	else
	{
		WRITE8PL(ea, privilege, value & 0xff);
		WRITE16PL(ea + 1, privilege, (value >> 8) & 0xffff);
		WRITE16PL(ea + 3, privilege, (value >> 24) & 0xffff);
		WRITE16PL(ea + 5, privilege, (value >> 40) & 0xffff);
		WRITE8PL(ea + 7, privilege, (value >> 56) & 0xff);
	}
}

/***********************************************************************************/

uint8_t i386_device::OR8(uint8_t dst, uint8_t src)
{
	uint8_t res = dst | src;
	m_CF = m_OF = 0;
	SetSZPF8(res);
	return res;
}
uint16_t i386_device::OR16(uint16_t dst, uint16_t src)
{
	uint16_t res = dst | src;
	m_CF = m_OF = 0;
	SetSZPF16(res);
	return res;
}
uint32_t i386_device::OR32(uint32_t dst, uint32_t src)
{
	uint32_t res = dst | src;
	m_CF = m_OF = 0;
	SetSZPF32(res);
	return res;
}

uint8_t i386_device::AND8(uint8_t dst, uint8_t src)
{
	uint8_t res = dst & src;
	m_CF = m_OF = 0;
	SetSZPF8(res);
	return res;
}
uint16_t i386_device::AND16(uint16_t dst, uint16_t src)
{
	uint16_t res = dst & src;
	m_CF = m_OF = 0;
	SetSZPF16(res);
	return res;
}
uint32_t i386_device::AND32(uint32_t dst, uint32_t src)
{
	uint32_t res = dst & src;
	m_CF = m_OF = 0;
	SetSZPF32(res);
	return res;
}

uint8_t i386_device::XOR8(uint8_t dst, uint8_t src)
{
	uint8_t res = dst ^ src;
	m_CF = m_OF = 0;
	SetSZPF8(res);
	return res;
}
uint16_t i386_device::XOR16(uint16_t dst, uint16_t src)
{
	uint16_t res = dst ^ src;
	m_CF = m_OF = 0;
	SetSZPF16(res);
	return res;
}
uint32_t i386_device::XOR32(uint32_t dst, uint32_t src)
{
	uint32_t res = dst ^ src;
	m_CF = m_OF = 0;
	SetSZPF32(res);
	return res;
}

#define SUB8(dst, src) SBB8(dst, src, 0)
uint8_t i386_device::SBB8(uint8_t dst, uint8_t src, uint8_t b)
{
	uint16_t res = (uint16_t)dst - (uint16_t)src - (uint8_t)b;
	SetCF8(res);
	SetOF_Sub8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}

#define SUB16(dst, src) SBB16(dst, src, 0)
uint16_t i386_device::SBB16(uint16_t dst, uint16_t src, uint16_t b)
{
	uint32_t res = (uint32_t)dst - (uint32_t)src - (uint32_t)b;
	SetCF16(res);
	SetOF_Sub16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}

#define SUB32(dst, src) SBB32(dst, src, 0)
uint32_t i386_device::SBB32(uint32_t dst, uint32_t src, uint32_t b)
{
	uint64_t res = (uint64_t)dst - (uint64_t)src - (uint64_t) b;
	SetCF32(res);
	SetOF_Sub32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}

#define ADD8(dst, src) ADC8(dst, src, 0)
uint8_t i386_device::ADC8(uint8_t dst, uint8_t src, uint8_t c)
{
	uint16_t res = (uint16_t)dst + (uint16_t)src + (uint16_t)c;
	SetCF8(res);
	SetOF_Add8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}

#define ADD16(dst, src) ADC16(dst, src, 0)
uint16_t i386_device::ADC16(uint16_t dst, uint16_t src, uint8_t c)
{
	uint32_t res = (uint32_t)dst + (uint32_t)src + (uint32_t)c;
	SetCF16(res);
	SetOF_Add16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}

#define ADD32(dst, src) ADC32(dst, src, 0)
uint32_t i386_device::ADC32(uint32_t dst, uint32_t src, uint32_t c)
{
	uint64_t res = (uint64_t)dst + (uint64_t)src + (uint64_t) c;
	SetCF32(res);
	SetOF_Add32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}

uint8_t i386_device::INC8(uint8_t dst)
{
	uint16_t res = (uint16_t)dst + 1;
	SetOF_Add8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}
uint16_t i386_device::INC16(uint16_t dst)
{
	uint32_t res = (uint32_t)dst + 1;
	SetOF_Add16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}
uint32_t i386_device::INC32(uint32_t dst)
{
	uint64_t res = (uint64_t)dst + 1;
	SetOF_Add32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}

uint8_t i386_device::DEC8(uint8_t dst)
{
	uint16_t res = (uint16_t)dst - 1;
	SetOF_Sub8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (uint8_t)res;
}
uint16_t i386_device::DEC16(uint16_t dst)
{
	uint32_t res = (uint32_t)dst - 1;
	SetOF_Sub16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (uint16_t)res;
}
uint32_t i386_device::DEC32(uint32_t dst)
{
	uint64_t res = (uint64_t)dst - 1;
	SetOF_Sub32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (uint32_t)res;
}



void i386_device::PUSH16(uint16_t value)
{
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 2;
		ea = i386_translate(SS, new_esp, 1);
		WRITE16(ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 2) & 0xffff;
		ea = i386_translate(SS, new_esp, 1);
		WRITE16(ea, value );
		REG16(SP) = new_esp;
	}
}
void i386_device::PUSH32(uint32_t value)
{
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate(SS, new_esp, 1);
		WRITE32(ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate(SS, new_esp, 1);
		WRITE32(ea, value );
		REG16(SP) = new_esp;
	}
}

void i386_device::PUSH32SEG(uint32_t value)
{
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate(SS, new_esp, 1);
		((m_cpu_version & 0xf00) == 0x300) ? WRITE16(ea, value) : WRITE32(ea, value ); // 486 also?
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate(SS, new_esp, 1);
		((m_cpu_version & 0xf00) == 0x300) ? WRITE16(ea, value) : WRITE32(ea, value );
		REG16(SP) = new_esp;
	}
}

void i386_device::PUSH8(uint8_t value)
{
	if( m_operand_size ) {
		PUSH32((int32_t)(int8_t)value);
	} else {
		PUSH16((int16_t)(int8_t)value);
	}
}

uint8_t i386_device::POP8()
{
	uint8_t value;
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 1;
		ea = i386_translate(SS, new_esp - 1, 0);
		value = READ8(ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 1;
		ea = i386_translate(SS, (new_esp - 1) & 0xffff, 0);
		value = READ8(ea );
		REG16(SP) = new_esp;
	}
	return value;
}
uint16_t i386_device::POP16()
{
	uint16_t value;
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 2;
		ea = i386_translate(SS, new_esp - 2, 0);
		value = READ16(ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 2;
		ea = i386_translate(SS, (new_esp - 2) & 0xffff, 0);
		value = READ16(ea );
		REG16(SP) = new_esp;
	}
	return value;
}
uint32_t i386_device::POP32()
{
	uint32_t value;
	uint32_t ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 4;
		ea = i386_translate(SS, new_esp - 4, 0);
		value = READ32(ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 4;
		ea = i386_translate(SS, (new_esp - 4) & 0xffff, 0);
		value = READ32(ea );
		REG16(SP) = new_esp;
	}
	return value;
}

void i386_device::BUMP_SI(int adjustment)
{
	if ( m_address_size )
		REG32(ESI) += ((m_DF) ? -adjustment : +adjustment);
	else
		REG16(SI) += ((m_DF) ? -adjustment : +adjustment);
}

void i386_device::BUMP_DI(int adjustment)
{
	if ( m_address_size )
		REG32(EDI) += ((m_DF) ? -adjustment : +adjustment);
	else
		REG16(DI) += ((m_DF) ? -adjustment : +adjustment);
}

void i386_device::CYCLES(int x)
{
	if (PROTECTED_MODE)
	{
		m_cycles -= m_cycle_table_pm[x];
	}
	else
	{
		m_cycles -= m_cycle_table_rm[x];
	}
}

void i386_device::CYCLES_RM(int modrm, int r, int m)
{
	if (modrm >= 0xc0)
	{
		if (PROTECTED_MODE)
		{
			m_cycles -= m_cycle_table_pm[r];
		}
		else
		{
			m_cycles -= m_cycle_table_rm[r];
		}
	}
	else
	{
		if (PROTECTED_MODE)
		{
			m_cycles -= m_cycle_table_pm[m];
		}
		else
		{
			m_cycles -= m_cycle_table_rm[m];
		}
	}
}

/**********************************************************************************/

void i386_device::check_ioperm(offs_t port, uint8_t mask)
{
	uint8_t IOPL, map;
	uint16_t IOPB;
	uint32_t address;

	if(!PROTECTED_MODE)
		return;

	IOPL = m_IOP1 | (m_IOP2 << 1);
	if(!V8086_MODE && (m_CPL <= IOPL))
		return;

	if((m_task.limit < 0x67) || ((m_task.flags & 0xd) != 9))
		FAULT_THROW(FAULT_GP,0);

	address = m_task.base;
	IOPB = READ16PL(address+0x66,0);
	if((IOPB+(port/8)) > m_task.limit)
		FAULT_THROW(FAULT_GP,0);

	map = READ8PL(address+IOPB+(port/8),0);
	map >>= (port%8);
	if(map & mask)
		FAULT_THROW(FAULT_GP,0);
}

uint8_t i386_device::READPORT8(offs_t port)
{
	check_ioperm(port, 1);
	return m_io->read_byte(port);
}

void i386_device::WRITEPORT8(offs_t port, uint8_t value)
{
	check_ioperm(port, 1);
	m_io->write_byte(port, value);
}

uint16_t i386_device::READPORT16(offs_t port)
{
	uint16_t value;

	switch (port & 3)
	{
	case 0:
	case 2:
	default:
		check_ioperm(port, 3);
		value = m_io->read_word(port);
		break;

	case 1:
		check_ioperm(port, 3);
		value = m_io->read_dword(port - 1, 0x00ffff00) >> 8;
		break;

	case 3:
		value = READPORT8(port);
		value |= (READPORT8(port + 1) << 8);
		break;
	}

	return value;
}

void i386_device::WRITEPORT16(offs_t port, uint16_t value)
{
	switch (port & 3)
	{
	case 0:
	case 2:
		check_ioperm(port, 3);
		m_io->write_word(port, value);
		break;

	case 1:
		check_ioperm(port, 3);
		m_io->write_dword(port - 1, value << 8, 0x00ffff00);
		break;

	case 3:
		WRITEPORT8(port, value & 0xff);
		WRITEPORT8(port + 1, (value >> 8) & 0xff);
		break;
	}
}

uint32_t i386_device::READPORT32(offs_t port)
{
	uint32_t value;

	switch (port & 3)
	{
	case 0:
	default:
		check_ioperm(port, 0xf);
		value = m_io->read_dword(port);
		break;

	case 1:
		check_ioperm(port, 7);
		value = m_io->read_dword(port - 1, 0xffffff00) >> 8;
		value |= READPORT8(port + 3) << 24;
		break;

	case 2:
		value = READPORT16(port);
		value |= READPORT16(port + 2) << 16;
		break;

	case 3:
		value = READPORT8(port);
		check_ioperm(port + 1, 7);
		value |= m_io->read_dword(port + 1, 0x00ffffff) << 8;
		break;
	}

	return value;
}

void i386_device::WRITEPORT32(offs_t port, uint32_t value)
{
	switch (port & 3)
	{
	case 0:
		check_ioperm(port, 0xf);
		m_io->write_dword(port, value);
		break;

	case 1:
		check_ioperm(port, 7);
		m_io->write_dword(port - 1, value << 8, 0xffffff00);
		WRITEPORT8(port + 3, (value >> 24) & 0xff);
		break;

	case 2:
		WRITEPORT16(port, value & 0xffff);
		WRITEPORT16(port + 2, (value >> 16) & 0xffff);
		break;

	case 3:
		WRITEPORT8(port, value & 0xff);
		check_ioperm(port + 1, 7);
		m_io->write_dword(port + 1, value >> 8, 0x00ffffff);
		break;
	}
}

uint16_t i386sx_device::READPORT16(offs_t port)
{
	if (port & 1)
	{
		uint16_t value = READPORT8(port);
		value |= (READPORT8(port + 1) << 8);
		return value;
	}
	else
	{
		check_ioperm(port, 3);
		return m_io->read_word(port);
	}
}

void i386sx_device::WRITEPORT16(offs_t port, uint16_t value)
{
	if (port & 1)
	{
		WRITEPORT8(port, value & 0xff);
		WRITEPORT8(port + 1, (value >> 8) & 0xff);
	}
	else
	{
		check_ioperm(port, 3);
		m_io->write_word(port, value);
	}
}

uint32_t i386sx_device::READPORT32(offs_t port)
{
	if (port & 1)
	{
		uint32_t value = READPORT8(port);
		value |= (READPORT16(port + 1) << 8);
		value |= (READPORT8(port + 3) << 24);
		return value;
	}
	else
	{
		uint16_t value = READPORT16(port);
		value |= (READPORT16(port + 2) << 16);
		return value;
	}
}

void i386sx_device::WRITEPORT32(offs_t port, uint32_t value)
{
	if (port & 1)
	{
		WRITEPORT8(port, value & 0xff);
		WRITEPORT16(port + 1, (value >> 8) & 0xffff);
		WRITEPORT8(port + 3, (value >> 24) & 0xff);
	}
	else
	{
		WRITEPORT16(port, value & 0xffff);
		WRITEPORT16((port + 2), (value >> 16) & 0xffff);
	}
}

/***********************************************************************************/

uint32_t i386_device::get_flags() const
{
	uint32_t f = 0x2;
	f |= m_CF;
	f |= m_PF << 2;
	f |= m_AF << 4;
	f |= m_ZF << 6;
	f |= m_SF << 7;
	f |= m_TF << 8;
	f |= m_IF << 9;
	f |= m_DF << 10;
	f |= m_OF << 11;
	f |= m_IOP1 << 12;
	f |= m_IOP2 << 13;
	f |= m_NT << 14;
	f |= m_RF << 16;
	f |= m_VM << 17;
	f |= m_AC << 18;
	f |= m_VIF << 19;
	f |= m_VIP << 20;
	f |= m_ID << 21;
	return (m_eflags & ~m_eflags_mask) | (f & m_eflags_mask);
}

void i386_device::set_flags(uint32_t f )
{
	f &= m_eflags_mask;
	m_CF = (f & 0x1) ? 1 : 0;
	m_PF = (f & 0x4) ? 1 : 0;
	m_AF = (f & 0x10) ? 1 : 0;
	m_ZF = (f & 0x40) ? 1 : 0;
	m_SF = (f & 0x80) ? 1 : 0;
	m_TF = (f & 0x100) ? 1 : 0;
	m_IF = (f & 0x200) ? 1 : 0;
	m_DF = (f & 0x400) ? 1 : 0;
	m_OF = (f & 0x800) ? 1 : 0;
	m_IOP1 = (f & 0x1000) ? 1 : 0;
	m_IOP2 = (f & 0x2000) ? 1 : 0;
	m_NT = (f & 0x4000) ? 1 : 0;
	m_RF = (f & 0x10000) ? 1 : 0;
	m_VM = (f & 0x20000) ? 1 : 0;
	m_AC = (f & 0x40000) ? 1 : 0;
	m_VIF = (f & 0x80000) ? 1 : 0;
	m_VIP = (f & 0x100000) ? 1 : 0;
	m_ID = (f & 0x200000) ? 1 : 0;
	m_eflags = f;
}

void i386_device::sib_byte(uint8_t mod, uint32_t* out_ea, uint8_t* out_segment)
{
	uint32_t ea = 0;
	uint8_t segment = 0;
	uint8_t scale, i, base;
	uint8_t sib = FETCH();
	scale = (sib >> 6) & 0x3;
	i = (sib >> 3) & 0x7;
	base = sib & 0x7;

	switch( base )
	{
		case 0: ea = REG32(EAX); segment = DS; break;
		case 1: ea = REG32(ECX); segment = DS; break;
		case 2: ea = REG32(EDX); segment = DS; break;
		case 3: ea = REG32(EBX); segment = DS; break;
		case 4: ea = REG32(ESP); segment = SS; break;
		case 5:
			if( mod == 0 ) {
				ea = FETCH32();
				segment = DS;
			} else if( mod == 1 ) {
				ea = REG32(EBP);
				segment = SS;
			} else if( mod == 2 ) {
				ea = REG32(EBP);
				segment = SS;
			}
			break;
		case 6: ea = REG32(ESI); segment = DS; break;
		case 7: ea = REG32(EDI); segment = DS; break;
	}
	switch( i )
	{
		case 0: ea += REG32(EAX) * (1 << scale); break;
		case 1: ea += REG32(ECX) * (1 << scale); break;
		case 2: ea += REG32(EDX) * (1 << scale); break;
		case 3: ea += REG32(EBX) * (1 << scale); break;
		case 4: break;
		case 5: ea += REG32(EBP) * (1 << scale); break;
		case 6: ea += REG32(ESI) * (1 << scale); break;
		case 7: ea += REG32(EDI) * (1 << scale); break;
	}
	*out_ea = ea;
	*out_segment = segment;
}

void i386_device::modrm_to_EA(uint8_t mod_rm, uint32_t* out_ea, uint8_t* out_segment)
{
	int8_t disp8;
	int16_t disp16;
	int32_t disp32;
	uint8_t mod = (mod_rm >> 6) & 0x3;
	uint8_t rm = mod_rm & 0x7;
	uint32_t ea;
	uint8_t segment;

	if( mod_rm >= 0xc0 )
		fatalerror("i386: Called modrm_to_EA with modrm value %02X!\n",mod_rm);


	if( m_address_size ) {
		switch( rm )
		{
			default:
			case 0: ea = REG32(EAX); segment = DS; break;
			case 1: ea = REG32(ECX); segment = DS; break;
			case 2: ea = REG32(EDX); segment = DS; break;
			case 3: ea = REG32(EBX); segment = DS; break;
			case 4: sib_byte(mod, &ea, &segment ); break;
			case 5:
				if( mod == 0 ) {
					ea = FETCH32(); segment = DS;
				} else {
					ea = REG32(EBP); segment = SS;
				}
				break;
			case 6: ea = REG32(ESI); segment = DS; break;
			case 7: ea = REG32(EDI); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH();
			ea += (int32_t)disp8;
		} else if( mod == 2 ) {
			disp32 = FETCH32();
			ea += disp32;
		}

		if( m_segment_prefix )
			segment = m_segment_override;

		*out_ea = ea;
		*out_segment = segment;

	} else {
		switch( rm )
		{
			default:
			case 0: ea = REG16(BX) + REG16(SI); segment = DS; break;
			case 1: ea = REG16(BX) + REG16(DI); segment = DS; break;
			case 2: ea = REG16(BP) + REG16(SI); segment = SS; break;
			case 3: ea = REG16(BP) + REG16(DI); segment = SS; break;
			case 4: ea = REG16(SI); segment = DS; break;
			case 5: ea = REG16(DI); segment = DS; break;
			case 6:
				if( mod == 0 ) {
					ea = FETCH16(); segment = DS;
				} else {
					ea = REG16(BP); segment = SS;
				}
				break;
			case 7: ea = REG16(BX); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH();
			ea += (int32_t)disp8;
		} else if( mod == 2 ) {
			disp16 = FETCH16();
			ea += (int32_t)disp16;
		}

		if( m_segment_prefix )
			segment = m_segment_override;

		*out_ea = ea & 0xffff;
		*out_segment = segment;
	}
}

uint32_t i386_device::GetNonTranslatedEA(uint8_t modrm,uint8_t *seg)
{
	uint8_t segment;
	uint32_t ea;
	modrm_to_EA(modrm, &ea, &segment );
	if(seg) *seg = segment;
	return ea;
}

uint32_t i386_device::GetEA(uint8_t modrm, int rwn)
{
	uint8_t segment;
	uint32_t ea;
	modrm_to_EA(modrm, &ea, &segment );
	return i386_translate(segment, ea, rwn );
}

void i386_device::i386_check_irq_line()
{
	if(!m_smm && m_smi)
	{
		enter_smm();
		return;
	}

	/* Check if the interrupts are enabled */
	if ( (m_irq_state) && m_IF )
	{
		m_cycles -= 2;
		i386_trap(standard_irq_callback(0), 1, 0);
	}
}

void i386_device::build_cycle_table()
{
	int i, j;
	for (j=0; j < X86_NUM_CPUS; j++)
	{
		cycle_table_rm[j] = std::make_unique<uint8_t[]>(CYCLES_NUM_OPCODES);
		cycle_table_pm[j] = std::make_unique<uint8_t[]>(CYCLES_NUM_OPCODES);

		for (i=0; i < sizeof(x86_cycle_table)/sizeof(X86_CYCLE_TABLE); i++)
		{
			int opcode = x86_cycle_table[i].op;
			cycle_table_rm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][0];
			cycle_table_pm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][1];
		}
	}
}

void i386_device::report_invalid_opcode()
{
#ifndef DEBUG_MISSING_OPCODE
	logerror("i386: Invalid opcode %02X at %08X %s\n", m_opcode, m_pc - 1, m_lock ? "with lock" : "");
#else
	logerror("Invalid opcode");
	for (int a = 0; a < m_opcode_bytes_length; a++)
		logerror(" %02X", m_opcode_bytes[a]);
	logerror(" at %08X %s\n", m_opcode_pc, m_lock ? "with lock" : "");
	logerror("Backtrace:\n");
	for (uint32_t i = 1; i < 16; i++)
		logerror("  %08X\n", m_opcode_addrs[(m_opcode_addrs_index - i) & 15]);
#endif
}

void i386_device::report_invalid_modrm(const char* opcode, uint8_t modrm)
{
#ifndef DEBUG_MISSING_OPCODE
	logerror("i386: Invalid %s modrm %01X at %08X\n", opcode, modrm, m_pc - 2);
#else
	logerror("Invalid %s modrm %01X", opcode, modrm);
	for (int a = 0; a < m_opcode_bytes_length; a++)
		logerror(" %02X", m_opcode_bytes[a]);
	logerror(" at %08X %s\n", m_opcode_pc, m_lock ? "with lock" : "");
	logerror("Backtrace:\n");
	for (uint32_t i = 1; i < 16; i++)
		logerror("  %08X\n", m_opcode_addrs[(m_opcode_addrs_index - i) & 15]);
#endif
	i386_trap(6, 0, 0);
}


#include "i386ops.hxx"
#include "i386op16.hxx"
#include "i386op32.hxx"
#include "i486ops.hxx"
#include "pentops.hxx"
#include "x87ops.hxx"
#include "cpuidmsrs.hxx"
#include "i386segs.hxx"


void i386_device::i386_decode_opcode()
{
	m_opcode = FETCH();

	if(m_lock && !m_lock_table[0][m_opcode])
		return i386_invalid();

	if( m_operand_size )
		(this->*m_opcode_table1_32[m_opcode])();
	else
		(this->*m_opcode_table1_16[m_opcode])();
}

/* Two-byte opcode 0f xx */
void i386_device::i386_decode_two_byte()
{
	m_opcode = FETCH();

	if(m_lock && !m_lock_table[1][m_opcode])
		return i386_invalid();

	if( m_operand_size )
		(this->*m_opcode_table2_32[m_opcode])();
	else
		(this->*m_opcode_table2_16[m_opcode])();
}

/* Three-byte opcode 0f 38 xx */
void i386_device::i386_decode_three_byte38()
{
	m_opcode = FETCH();

	if (m_operand_size)
		(this->*m_opcode_table338_32[m_opcode])();
	else
		(this->*m_opcode_table338_16[m_opcode])();
}

/* Three-byte opcode 0f 3a xx */
void i386_device::i386_decode_three_byte3a()
{
	m_opcode = FETCH();

	if (m_operand_size)
		(this->*m_opcode_table33a_32[m_opcode])();
	else
		(this->*m_opcode_table33a_16[m_opcode])();
}

/* Three-byte opcode prefix 66 0f xx */
void i386_device::i386_decode_three_byte66()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table366_32[m_opcode])();
	else
		(this->*m_opcode_table366_16[m_opcode])();
}

/* Three-byte opcode prefix f2 0f xx */
void i386_device::i386_decode_three_bytef2()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table3f2_32[m_opcode])();
	else
		(this->*m_opcode_table3f2_16[m_opcode])();
}

/* Three-byte opcode prefix f3 0f */
void i386_device::i386_decode_three_bytef3()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table3f3_32[m_opcode])();
	else
		(this->*m_opcode_table3f3_16[m_opcode])();
}

/* Four-byte opcode prefix 66 0f 38 xx */
void i386_device::i386_decode_four_byte3866()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table46638_32[m_opcode])();
	else
		(this->*m_opcode_table46638_16[m_opcode])();
}

/* Four-byte opcode prefix 66 0f 3a xx */
void i386_device::i386_decode_four_byte3a66()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4663a_32[m_opcode])();
	else
		(this->*m_opcode_table4663a_16[m_opcode])();
}

/* Four-byte opcode prefix f2 0f 38 xx */
void i386_device::i386_decode_four_byte38f2()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f238_32[m_opcode])();
	else
		(this->*m_opcode_table4f238_16[m_opcode])();
}

/* Four-byte opcode prefix f2 0f 3a xx */
void i386_device::i386_decode_four_byte3af2()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f23a_32[m_opcode])();
	else
		(this->*m_opcode_table4f23a_16[m_opcode])();
}

/* Four-byte opcode prefix f3 0f 38 xx */
void i386_device::i386_decode_four_byte38f3()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f338_32[m_opcode])();
	else
		(this->*m_opcode_table4f338_16[m_opcode])();
}


/*************************************************************************/

uint8_t i386_device::read8_debug(uint32_t ea, uint8_t *data)
{
	uint32_t address = ea;

	if(!i386_translate_address(TRANSLATE_DEBUG_MASK,&address,nullptr))
		return 0;

	address &= m_a20_mask;
	*data = m_program->read_byte(address);
	return 1;
}

uint32_t i386_device::i386_get_debug_desc(I386_SREG *seg)
{
	uint32_t base, limit, address;
	union { uint8_t b[8]; uint32_t w[2]; } data;
	uint8_t ret;
	int entry;

	if ( seg->selector & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = seg->selector & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return 0;

	address = entry + base;

	// todo: bigendian
	ret = read8_debug( address+0, &data.b[0] );
	ret += read8_debug( address+1, &data.b[1] );
	ret += read8_debug( address+2, &data.b[2] );
	ret += read8_debug( address+3, &data.b[3] );
	ret += read8_debug( address+4, &data.b[4] );
	ret += read8_debug( address+5, &data.b[5] );
	ret += read8_debug( address+6, &data.b[6] );
	ret += read8_debug( address+7, &data.b[7] );

	if(ret != 8)
		return 0;

	seg->flags = (data.w[1] >> 8) & 0xf0ff;
	seg->base = (data.w[1] & 0xff000000) | ((data.w[1] & 0xff) << 16) | ((data.w[0] >> 16) & 0xffff);
	seg->limit = (data.w[1] & 0xf0000) | (data.w[0] & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
	seg->valid = (seg->selector & ~3)?(true):(false);

	return seg->valid;
}

uint64_t i386_device::debug_segbase(int params, const uint64_t *param)
{
	uint32_t result;
	I386_SREG seg;

	if(param[0] > 65535)
		return 0;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		result = seg.base;
	}
	else
	{
		result = param[0] << 4;
	}
	return result;
}

uint64_t i386_device::debug_seglimit(int params, const uint64_t *param)
{
	uint32_t result = 0;
	I386_SREG seg;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		result = seg.limit;
	}
	return result;
}

uint64_t i386_device::debug_segofftovirt(int params, const uint64_t *param)
{
	uint32_t result;
	I386_SREG seg;

	if(param[0] > 65535)
		return 0;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		if((seg.flags & 0x0090) != 0x0090) // not system and present
			return 0;
		if((seg.flags & 0x0018) == 0x0010 && seg.flags & 0x0004) // expand down
		{
			if(param[1] <= seg.limit)
				return 0;
		}
		else
		{
			if(param[1] > seg.limit)
				return 0;
		}
		result = seg.base+param[1];
	}
	else
	{
		if(param[1] > 65535)
			return 0;

		result = (param[0] << 4) + param[1];
	}
	return result;
}

uint64_t i386_device::debug_virttophys(int params, const uint64_t *param)
{
	uint32_t result = param[0];

	if(!i386_translate_address(TRANSLATE_DEBUG_MASK,&result,nullptr))
		return 0;
	return result;
}

uint64_t i386_device::debug_cacheflush(int params, const uint64_t *param)
{
	uint32_t option;
	bool invalidate;
	bool clean;

	if (params > 0)
		option = param[0];
	else
		option = 0;
	invalidate = (option & 1) != 0;
	clean = (option & 2) != 0;
	cache_writeback();
	if (invalidate)
		cache_invalidate();
	if (clean)
		cache_clean();
	return 0;
}

void i386_device::device_debug_setup()
{
	using namespace std::placeholders;
	debug()->symtable().add("segbase", 1, 1, std::bind(&i386_device::debug_segbase, this, _1, _2));
	debug()->symtable().add("seglimit", 1, 1, std::bind(&i386_device::debug_seglimit, this, _1, _2));
	debug()->symtable().add("segofftovirt", 2, 2, std::bind(&i386_device::debug_segofftovirt, this, _1, _2));
	debug()->symtable().add("virttophys", 1, 1, std::bind(&i386_device::debug_virttophys, this, _1, _2));
	debug()->symtable().add("cacheflush", 0, 1, std::bind(&i386_device::debug_cacheflush, this, _1, _2));
}

/*************************************************************************/

void i386_device::i386_postload()
{
	int i;
	for (i = 0; i < 6; i++)
		i386_load_segment_descriptor(i);
	CHANGE_PC(m_eip);
}

void i386_device::i386_common_init()
{
	int i, j;
	static const int regs8[8] = {AL,CL,DL,BL,AH,CH,DH,BH};
	static const int regs16[8] = {AX,CX,DX,BX,SP,BP,SI,DI};
	static const int regs32[8] = {EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};

	assert((sizeof(XMM_REG)/sizeof(double)) == 2);

	build_cycle_table();

	for( i=0; i < 256; i++ ) {
		int c=0;
		for( j=0; j < 8; j++ ) {
			if( i & (1 << j) )
				c++;
		}
		i386_parity_table[i] = ~(c & 0x1) & 0x1;
	}

	for( i=0; i < 256; i++ ) {
		i386_MODRM_table[i].reg.b = regs8[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.w = regs16[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.d = regs32[(i >> 3) & 0x7];

		i386_MODRM_table[i].rm.b = regs8[i & 0x7];
		i386_MODRM_table[i].rm.w = regs16[i & 0x7];
		i386_MODRM_table[i].rm.d = regs32[i & 0x7];
	}

	m_program = &space(AS_PROGRAM);
	if(m_program->data_width() == 16) {
		// for the 386sx
		m_program->cache(macache16);
	} else {
		m_program->cache(macache32);
	}

	m_io = &space(AS_IO);
	m_smi = false;
	m_debugger_temp = 0;
	m_lock = false;

	zero_state();

	save_item(NAME(m_reg.d));
	save_item(STRUCT_MEMBER(m_sreg, selector));
	save_item(STRUCT_MEMBER(m_sreg, base));
	save_item(STRUCT_MEMBER(m_sreg, limit));
	save_item(STRUCT_MEMBER(m_sreg, flags));
	save_item(STRUCT_MEMBER(m_sreg, d));
	save_item(NAME(m_eip));
	save_item(NAME(m_prev_eip));

	save_item(NAME(m_CF));
	save_item(NAME(m_DF));
	save_item(NAME(m_SF));
	save_item(NAME(m_OF));
	save_item(NAME(m_ZF));
	save_item(NAME(m_PF));
	save_item(NAME(m_AF));
	save_item(NAME(m_IF));
	save_item(NAME(m_TF));
	save_item(NAME(m_IOP1));
	save_item(NAME(m_IOP2));
	save_item(NAME(m_NT));
	save_item(NAME(m_RF));
	save_item(NAME(m_VM));
	save_item(NAME(m_AC));
	save_item(NAME(m_VIF));
	save_item(NAME(m_VIP));
	save_item(NAME(m_ID));

	save_item(NAME(m_CPL));

	save_item(NAME(m_auto_clear_RF));
	save_item(NAME(m_performed_intersegment_jump));

	save_item(NAME(m_cr));
	save_item(NAME(m_dr));
	save_item(NAME(m_tr));

	save_item(NAME(m_idtr.base));
	save_item(NAME(m_idtr.limit));
	save_item(NAME(m_gdtr.base));
	save_item(NAME(m_gdtr.limit));
	save_item(NAME(m_task.base));
	save_item(NAME(m_task.segment));
	save_item(NAME(m_task.limit));
	save_item(NAME(m_task.flags));
	save_item(NAME(m_ldtr.base));
	save_item(NAME(m_ldtr.segment));
	save_item(NAME(m_ldtr.limit));
	save_item(NAME(m_ldtr.flags));

	save_item(NAME(m_segment_override));

	save_item(NAME(m_irq_state));
	save_item(NAME(m_a20_mask));

	save_item(NAME(m_mxcsr));

	save_item(NAME(m_smm));
	save_item(NAME(m_smi));
	save_item(NAME(m_smi_latched));
	save_item(NAME(m_nmi_masked));
	save_item(NAME(m_nmi_latched));
	save_item(NAME(m_smbase));
	save_item(NAME(m_lock));
	machine().save().register_postload(save_prepost_delegate(FUNC(i386_device::i386_postload), this));

	m_smiact.resolve_safe();
	m_ferr_handler.resolve_safe();
	m_ferr_handler(0);

	set_icountptr(m_cycles);
	m_notifier = m_program->add_change_notifier([this] (read_or_write mode) { dri_changed(); });
}

void i386_device::device_start()
{
	i386_common_init();

	build_opcode_table(OP_I386);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_I386].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_I386].get();

	register_state_i386();
}

void i386_device::register_state_i386()
{
	state_add( I386_PC,         "PC", m_pc).formatstr("%08X");
	state_add( I386_EIP,        "EIP", m_eip).callimport().formatstr("%08X");
	state_add( I386_AL,         "~AL", REG8(AL)).formatstr("%02X");
	state_add( I386_AH,         "~AH", REG8(AH)).formatstr("%02X");
	state_add( I386_BL,         "~BL", REG8(BL)).formatstr("%02X");
	state_add( I386_BH,         "~BH", REG8(BH)).formatstr("%02X");
	state_add( I386_CL,         "~CL", REG8(CL)).formatstr("%02X");
	state_add( I386_CH,         "~CH", REG8(CH)).formatstr("%02X");
	state_add( I386_DL,         "~DL", REG8(DL)).formatstr("%02X");
	state_add( I386_DH,         "~DH", REG8(DH)).formatstr("%02X");
	state_add( I386_AX,         "~AX", REG16(AX)).formatstr("%04X");
	state_add( I386_BX,         "~BX", REG16(BX)).formatstr("%04X");
	state_add( I386_CX,         "~CX", REG16(CX)).formatstr("%04X");
	state_add( I386_DX,         "~DX", REG16(DX)).formatstr("%04X");
	state_add( I386_SI,         "~SI", REG16(SI)).formatstr("%04X");
	state_add( I386_DI,         "~DI", REG16(DI)).formatstr("%04X");
	state_add( I386_BP,         "~BP", REG16(BP)).formatstr("%04X");
	state_add( I386_SP,         "~SP", REG16(SP)).formatstr("%04X");
	state_add( I386_IP,         "~IP", m_debugger_temp).mask(0xffff).callimport().callexport().formatstr("%04X");
	state_add( I386_EAX,        "EAX", m_reg.d[EAX]).formatstr("%08X");
	state_add( I386_EBX,        "EBX", m_reg.d[EBX]).formatstr("%08X");
	state_add( I386_ECX,        "ECX", m_reg.d[ECX]).formatstr("%08X");
	state_add( I386_EDX,        "EDX", m_reg.d[EDX]).formatstr("%08X");
	state_add( I386_EBP,        "EBP", m_reg.d[EBP]).formatstr("%08X");
	state_add( I386_ESP,        "ESP", m_reg.d[ESP]).formatstr("%08X");
	state_add( I386_ESI,        "ESI", m_reg.d[ESI]).formatstr("%08X");
	state_add( I386_EDI,        "EDI", m_reg.d[EDI]).formatstr("%08X");
	state_add( I386_EFLAGS,     "EFLAGS", m_eflags).formatstr("%08X");
	state_add( I386_CS,         "CS", m_sreg[CS].selector).callimport().formatstr("%04X");
	state_add( I386_CS_BASE,    "CSBASE", m_sreg[CS].base).formatstr("%08X");
	state_add( I386_CS_LIMIT,   "CSLIMIT", m_sreg[CS].limit).formatstr("%08X");
	state_add( I386_CS_FLAGS,   "CSFLAGS", m_sreg[CS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_SS,         "SS", m_sreg[SS].selector).callimport().formatstr("%04X");
	state_add( I386_SS_BASE,    "SSBASE", m_sreg[SS].base).formatstr("%08X");
	state_add( I386_SS_LIMIT,   "SSLIMIT", m_sreg[SS].limit).formatstr("%08X");
	state_add( I386_SS_FLAGS,   "SSFLAGS", m_sreg[SS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_DS,         "DS", m_sreg[DS].selector).callimport().formatstr("%04X");
	state_add( I386_DS_BASE,    "DSBASE", m_sreg[DS].base).formatstr("%08X");
	state_add( I386_DS_LIMIT,   "DSLIMIT", m_sreg[DS].limit).formatstr("%08X");
	state_add( I386_DS_FLAGS,   "DSFLAGS", m_sreg[DS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_ES,         "ES", m_sreg[ES].selector).callimport().formatstr("%04X");
	state_add( I386_ES_BASE,    "ESBASE", m_sreg[ES].base).formatstr("%08X");
	state_add( I386_ES_LIMIT,   "ESLIMIT", m_sreg[ES].limit).formatstr("%08X");
	state_add( I386_ES_FLAGS,   "ESFLAGS", m_sreg[ES].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_FS,         "FS", m_sreg[FS].selector).callimport().formatstr("%04X");
	state_add( I386_FS_BASE,    "FSBASE", m_sreg[FS].base).formatstr("%08X");
	state_add( I386_FS_LIMIT,   "FSLIMIT", m_sreg[FS].limit).formatstr("%08X");
	state_add( I386_FS_FLAGS,   "FSFLAGS", m_sreg[FS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_GS,         "GS", m_sreg[GS].selector).callimport().formatstr("%04X");
	state_add( I386_GS_BASE,    "GSBASE", m_sreg[GS].base).formatstr("%08X");
	state_add( I386_GS_LIMIT,   "GSLIMIT", m_sreg[GS].limit).formatstr("%08X");
	state_add( I386_GS_FLAGS,   "GSFLAGS", m_sreg[GS].flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_CR0,        "CR0", m_cr[0]).formatstr("%08X");
	state_add( I386_CR1,        "CR1", m_cr[1]).formatstr("%08X");
	state_add( I386_CR2,        "CR2", m_cr[2]).formatstr("%08X");
	state_add( I386_CR3,        "CR3", m_cr[3]).formatstr("%08X");
	state_add( I386_CR4,        "CR4", m_cr[4]).formatstr("%08X");
	state_add( I386_DR0,        "DR0", m_dr[0]).formatstr("%08X");
	state_add( I386_DR1,        "DR1", m_dr[1]).formatstr("%08X");
	state_add( I386_DR2,        "DR2", m_dr[2]).formatstr("%08X");
	state_add( I386_DR3,        "DR3", m_dr[3]).formatstr("%08X");
	state_add( I386_DR4,        "DR4", m_dr[4]).formatstr("%08X");
	state_add( I386_DR5,        "DR5", m_dr[5]).formatstr("%08X");
	state_add( I386_DR6,        "DR6", m_dr[6]).formatstr("%08X");
	state_add( I386_DR7,        "DR7", m_dr[7]).formatstr("%08X");
	state_add( I386_TR6,        "TR6", m_tr[6]).formatstr("%08X");
	state_add( I386_TR7,        "TR7", m_tr[7]).formatstr("%08X");
	state_add( I386_GDTR_BASE,  "GDTRBASE", m_gdtr.base).formatstr("%08X");
	state_add( I386_GDTR_LIMIT, "GDTRLIMIT", m_gdtr.limit).formatstr("%04X");
	state_add( I386_IDTR_BASE,  "IDTRBASE", m_idtr.base).formatstr("%08X");
	state_add( I386_IDTR_LIMIT, "IDTRLIMIT", m_idtr.limit).formatstr("%04X");
	state_add( I386_LDTR,       "LDTR", m_ldtr.segment).formatstr("%04X");
	state_add( I386_LDTR_BASE,  "LDTRBASE", m_ldtr.base).formatstr("%08X");
	state_add( I386_LDTR_LIMIT, "LDTRLIMIT", m_ldtr.limit).formatstr("%08X");
	state_add( I386_LDTR_FLAGS, "LDTRFLAGS", m_ldtr.flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_TR,         "TR", m_task.segment).formatstr("%04X");
	state_add( I386_TR_BASE,    "TRBASE", m_task.base).formatstr("%08X");
	state_add( I386_TR_LIMIT,   "TRLIMIT", m_task.limit).formatstr("%08X");
	state_add( I386_TR_FLAGS,   "TRFLAGS", m_task.flags).mask(0xf0ff).formatstr("%04X");
	state_add( I386_CPL,        "CPL", m_CPL).formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%32s").noshow();
}

void i386_device::register_state_i386_x87()
{
	register_state_i386();

	state_add( X87_CTRL,   "x87_CW", m_x87_cw).formatstr("%04X");
	state_add( X87_STATUS, "x87_SW", m_x87_sw).formatstr("%04X");
	state_add( X87_TAG,    "x87_TAG", m_x87_tw).formatstr("%04X");
	state_add( X87_ST0,    "ST0", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST1,    "ST1", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST2,    "ST2", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST3,    "ST3", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST4,    "ST4", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST5,    "ST5", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST6,    "ST6", m_debugger_temp ).formatstr("%15s");
	state_add( X87_ST7,    "ST7", m_debugger_temp ).formatstr("%15s");
}

void i386_device::register_state_i386_x87_xmm()
{
	register_state_i386_x87();

	state_add( SSE_XMM0, "XMM0", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM1, "XMM1", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM2, "XMM2", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM3, "XMM3", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM4, "XMM4", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM5, "XMM5", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM6, "XMM6", m_debugger_temp ).formatstr("%32s");
	state_add( SSE_XMM7, "XMM7", m_debugger_temp ).formatstr("%32s");

}

void i386_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case I386_EIP:
			CHANGE_PC(m_eip);
			break;
		case I386_IP:
			m_eip = ( m_eip & ~0xffff ) | ( m_debugger_temp & 0xffff);
			CHANGE_PC(m_eip);
			break;
		case I386_CS:
			i386_load_segment_descriptor(CS);
			break;
		case I386_SS:
			i386_load_segment_descriptor(SS);
			break;
		case I386_DS:
			i386_load_segment_descriptor(DS);
			break;
		case I386_ES:
			i386_load_segment_descriptor(ES);
			break;
		case I386_FS:
			i386_load_segment_descriptor(FS);
			break;
		case I386_GS:
			i386_load_segment_descriptor(GS);
			break;
	}
}

void i386_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case I386_IP:
			m_debugger_temp = m_eip & 0xffff;
			break;
	}
}

void i386_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%08X %s%s%d%s%s%s%s%s%s%s%s%s",
				get_flags(),
				m_RF ? "R" : "r",
				m_NT ? " N " : " n ",
				m_IOP2 << 1 | m_IOP1,
				m_OF ? " O" : " o",
				m_DF ? " D" : " d",
				m_IF ? " I" : " i",
				m_TF ? " T" : " t",
				m_SF ? " S" : " s",
				m_ZF ? " Z" : " z",
				m_AF ? " A" : " a",
				m_PF ? " P" : " p",
				m_CF ? " C" : " c");
			break;
		case X87_ST0:
			str = string_format("%f", fx80_to_double(ST(0)));
			break;
		case X87_ST1:
			str = string_format("%f", fx80_to_double(ST(1)));
			break;
		case X87_ST2:
			str = string_format("%f", fx80_to_double(ST(2)));
			break;
		case X87_ST3:
			str = string_format("%f", fx80_to_double(ST(3)));
			break;
		case X87_ST4:
			str = string_format("%f", fx80_to_double(ST(4)));
			break;
		case X87_ST5:
			str = string_format("%f", fx80_to_double(ST(5)));
			break;
		case X87_ST6:
			str = string_format("%f", fx80_to_double(ST(6)));
			break;
		case X87_ST7:
			str = string_format("%f", fx80_to_double(ST(7)));
			break;
		case SSE_XMM0:
			str = string_format("%08x%08x%08x%08x", XMM(0).d[3], XMM(0).d[2], XMM(0).d[1], XMM(0).d[0]);
			break;
		case SSE_XMM1:
			str = string_format("%08x%08x%08x%08x", XMM(1).d[3], XMM(1).d[2], XMM(1).d[1], XMM(1).d[0]);
			break;
		case SSE_XMM2:
			str = string_format("%08x%08x%08x%08x", XMM(2).d[3], XMM(2).d[2], XMM(2).d[1], XMM(2).d[0]);
			break;
		case SSE_XMM3:
			str = string_format("%08x%08x%08x%08x", XMM(3).d[3], XMM(3).d[2], XMM(3).d[1], XMM(3).d[0]);
			break;
		case SSE_XMM4:
			str = string_format("%08x%08x%08x%08x", XMM(4).d[3], XMM(4).d[2], XMM(4).d[1], XMM(4).d[0]);
			break;
		case SSE_XMM5:
			str = string_format("%08x%08x%08x%08x", XMM(5).d[3], XMM(5).d[2], XMM(5).d[1], XMM(5).d[0]);
			break;
		case SSE_XMM6:
			str = string_format("%08x%08x%08x%08x", XMM(6).d[3], XMM(6).d[2], XMM(6).d[1], XMM(6).d[0]);
			break;
		case SSE_XMM7:
			str = string_format("%08x%08x%08x%08x", XMM(7).d[3], XMM(7).d[2], XMM(7).d[1], XMM(7).d[0]);
			break;
	}
	float_exception_flags = 0; // kill any float exceptions that occur here
}

void i386_device::build_opcode_table(uint32_t features)
{
	int i;
	for (i=0; i < 256; i++)
	{
		m_opcode_table1_16[i] = &i386_device::i386_invalid;
		m_opcode_table1_32[i] = &i386_device::i386_invalid;
		m_opcode_table2_16[i] = &i386_device::i386_invalid;
		m_opcode_table2_32[i] = &i386_device::i386_invalid;
		m_opcode_table366_16[i] = &i386_device::i386_invalid;
		m_opcode_table366_32[i] = &i386_device::i386_invalid;
		m_opcode_table3f2_16[i] = &i386_device::i386_invalid;
		m_opcode_table3f2_32[i] = &i386_device::i386_invalid;
		m_opcode_table3f3_16[i] = &i386_device::i386_invalid;
		m_opcode_table3f3_32[i] = &i386_device::i386_invalid;
		m_lock_table[0][i] = false;
		m_lock_table[1][i] = false;
	}

	for (i=0; i < sizeof(s_x86_opcode_table)/sizeof(X86_OPCODE); i++)
	{
		const X86_OPCODE *op = &s_x86_opcode_table[i];

		if ((op->flags & features))
		{
			if (op->flags & OP_2BYTE)
			{
				m_opcode_table2_32[op->opcode] = op->handler32;
				m_opcode_table2_16[op->opcode] = op->handler16;
				m_opcode_table366_32[op->opcode] = op->handler32;
				m_opcode_table366_16[op->opcode] = op->handler16;
				m_lock_table[1][op->opcode] = op->lockable;
			}
			else if (op->flags & OP_3BYTE66)
			{
				m_opcode_table366_32[op->opcode] = op->handler32;
				m_opcode_table366_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTEF2)
			{
				m_opcode_table3f2_32[op->opcode] = op->handler32;
				m_opcode_table3f2_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTEF3)
			{
				m_opcode_table3f3_32[op->opcode] = op->handler32;
				m_opcode_table3f3_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTE38)
			{
				m_opcode_table338_32[op->opcode] = op->handler32;
				m_opcode_table338_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTE3A)
			{
				m_opcode_table33a_32[op->opcode] = op->handler32;
				m_opcode_table33a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3866)
			{
				m_opcode_table46638_32[op->opcode] = op->handler32;
				m_opcode_table46638_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3A66)
			{
				m_opcode_table4663a_32[op->opcode] = op->handler32;
				m_opcode_table4663a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE38F2)
			{
				m_opcode_table4f238_32[op->opcode] = op->handler32;
				m_opcode_table4f238_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3AF2)
			{
				m_opcode_table4f23a_32[op->opcode] = op->handler32;
				m_opcode_table4f23a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE38F3)
			{
				m_opcode_table4f338_32[op->opcode] = op->handler32;
				m_opcode_table4f338_16[op->opcode] = op->handler16;
			}
			else
			{
				m_opcode_table1_32[op->opcode] = op->handler32;
				m_opcode_table1_16[op->opcode] = op->handler16;
				m_lock_table[0][op->opcode] = op->lockable;
			}
		}
	}
}

void i386_device::zero_state()
{
	memset( &m_reg, 0, sizeof(m_reg) );
	memset( m_sreg, 0, sizeof(m_sreg) );
	m_eip = 0;
	m_pc = 0;
	m_prev_eip = 0;
	m_eflags = 0;
	m_eflags_mask = 0;
	m_CF = 0;
	m_DF = 0;
	m_SF = 0;
	m_OF = 0;
	m_ZF = 0;
	m_PF = 0;
	m_AF = 0;
	m_IF = 0;
	m_TF = 0;
	m_IOP1 = 0;
	m_IOP2 = 0;
	m_NT = 0;
	m_RF = 0;
	m_VM = 0;
	m_AC = 0;
	m_VIF = 0;
	m_VIP = 0;
	m_ID = 0;
	m_CPL = 0;
	m_performed_intersegment_jump = 0;
	m_delayed_interrupt_enable = 0;
	memset( m_cr, 0, sizeof(m_cr) );
	memset( m_dr, 0, sizeof(m_dr) );
	memset( m_tr, 0, sizeof(m_tr) );
	memset( &m_gdtr, 0, sizeof(m_gdtr) );
	memset( &m_idtr, 0, sizeof(m_idtr) );
	memset( &m_task, 0, sizeof(m_task) );
	memset( &m_ldtr, 0, sizeof(m_ldtr) );
	m_ext = 0;
	m_halted = 0;
	m_operand_size = 0;
	m_xmm_operand_size = 0;
	m_address_size = 0;
	m_operand_prefix = 0;
	m_address_prefix = 0;
	m_segment_prefix = 0;
	m_segment_override = 0;
	m_cycles = 0;
	m_base_cycles = 0;
	m_opcode = 0;
	m_irq_state = 0;
	m_a20_mask = 0;
	m_cpuid_max_input_value_eax = 0;
	m_cpuid_id0 = 0;
	m_cpuid_id1 = 0;
	m_cpuid_id2 = 0;
	m_cpu_version = 0;
	m_feature_flags = 0;
	m_tsc = 0;
	m_perfctr[0] = m_perfctr[1] = 0;
	memset( m_x87_reg, 0, sizeof(m_x87_reg) );
	m_x87_cw = 0;
	m_x87_sw = 0;
	m_x87_tw = 0;
	m_x87_data_ptr = 0;
	m_x87_inst_ptr = 0;
	m_x87_opcode = 0;
	memset( m_sse_reg, 0, sizeof(m_sse_reg) );
	m_mxcsr = 0;
	m_smm = false;
	m_smi = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;
	m_smbase = 0;
	memset( m_opcode_bytes, 0, sizeof(m_opcode_bytes) );
	m_opcode_pc = 0;
	m_opcode_bytes_length = 0;
	memset(m_opcode_addrs, 0, sizeof(m_opcode_addrs));
	m_opcode_addrs_index = 0;
}

void i386_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x93;
	m_sreg[CS].valid    = true;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid =true;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	m_a20_mask = ~0;

	m_cr[0] = 0x7fffffe0; // reserved bits set to 1
	m_eflags = 0;
	m_eflags_mask = 0x00037fd7;
	m_eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 3 (386), Model 0 (DX), Stepping 8 (D1)
	REG32(EAX) = 0;
	REG32(EDX) = (3 << 8) | (0 << 4) | (8);
	m_cpu_version = REG32(EDX);

	m_CPL = 0;

	m_auto_clear_RF = true;

	CHANGE_PC(m_eip);
}

void i386_device::enter_smm()
{
	uint32_t smram_state = m_smbase + 0xfe00;
	uint32_t old_cr0 = m_cr[0];
	uint32_t old_flags = get_flags();

	m_cr[0] &= ~(0x8000000d);
	set_flags(2);
	if(!m_smiact.isnull())
		m_smiact(true);
	m_smm = true;
	m_smi_latched = false;

	// save state
	WRITE32(smram_state + SMRAM_SMBASE, m_smbase);
	WRITE32(smram_state + SMRAM_IP5_CR4, m_cr[4]);
	WRITE32(smram_state + SMRAM_IP5_ESLIM, m_sreg[ES].limit);
	WRITE32(smram_state + SMRAM_IP5_ESBASE, m_sreg[ES].base);
	WRITE32(smram_state + SMRAM_IP5_ESACC, m_sreg[ES].flags);
	WRITE32(smram_state + SMRAM_IP5_CSLIM, m_sreg[CS].limit);
	WRITE32(smram_state + SMRAM_IP5_CSBASE, m_sreg[CS].base);
	WRITE32(smram_state + SMRAM_IP5_CSACC, m_sreg[CS].flags);
	WRITE32(smram_state + SMRAM_IP5_SSLIM, m_sreg[SS].limit);
	WRITE32(smram_state + SMRAM_IP5_SSBASE, m_sreg[SS].base);
	WRITE32(smram_state + SMRAM_IP5_SSACC, m_sreg[SS].flags);
	WRITE32(smram_state + SMRAM_IP5_DSLIM, m_sreg[DS].limit);
	WRITE32(smram_state + SMRAM_IP5_DSBASE, m_sreg[DS].base);
	WRITE32(smram_state + SMRAM_IP5_DSACC, m_sreg[DS].flags);
	WRITE32(smram_state + SMRAM_IP5_FSLIM, m_sreg[FS].limit);
	WRITE32(smram_state + SMRAM_IP5_FSBASE, m_sreg[FS].base);
	WRITE32(smram_state + SMRAM_IP5_FSACC, m_sreg[FS].flags);
	WRITE32(smram_state + SMRAM_IP5_GSLIM, m_sreg[GS].limit);
	WRITE32(smram_state + SMRAM_IP5_GSBASE, m_sreg[GS].base);
	WRITE32(smram_state + SMRAM_IP5_GSACC, m_sreg[GS].flags);
	WRITE32(smram_state + SMRAM_IP5_LDTACC, m_ldtr.flags);
	WRITE32(smram_state + SMRAM_IP5_LDTLIM, m_ldtr.limit);
	WRITE32(smram_state + SMRAM_IP5_LDTBASE, m_ldtr.base);
	WRITE32(smram_state + SMRAM_IP5_GDTLIM, m_gdtr.limit);
	WRITE32(smram_state + SMRAM_IP5_GDTBASE, m_gdtr.base);
	WRITE32(smram_state + SMRAM_IP5_IDTLIM, m_idtr.limit);
	WRITE32(smram_state + SMRAM_IP5_IDTBASE, m_idtr.base);
	WRITE32(smram_state + SMRAM_IP5_TRLIM, m_task.limit);
	WRITE32(smram_state + SMRAM_IP5_TRBASE, m_task.base);
	WRITE32(smram_state + SMRAM_IP5_TRACC, m_task.flags);

	WRITE32(smram_state + SMRAM_ES, m_sreg[ES].selector);
	WRITE32(smram_state + SMRAM_CS, m_sreg[CS].selector);
	WRITE32(smram_state + SMRAM_SS, m_sreg[SS].selector);
	WRITE32(smram_state + SMRAM_DS, m_sreg[DS].selector);
	WRITE32(smram_state + SMRAM_FS, m_sreg[FS].selector);
	WRITE32(smram_state + SMRAM_GS, m_sreg[GS].selector);
	WRITE32(smram_state + SMRAM_LDTR, m_ldtr.segment);
	WRITE32(smram_state + SMRAM_TR, m_task.segment);

	WRITE32(smram_state + SMRAM_DR7, m_dr[7]);
	WRITE32(smram_state + SMRAM_DR6, m_dr[6]);
	WRITE32(smram_state + SMRAM_EAX, REG32(EAX));
	WRITE32(smram_state + SMRAM_ECX, REG32(ECX));
	WRITE32(smram_state + SMRAM_EDX, REG32(EDX));
	WRITE32(smram_state + SMRAM_EBX, REG32(EBX));
	WRITE32(smram_state + SMRAM_ESP, REG32(ESP));
	WRITE32(smram_state + SMRAM_EBP, REG32(EBP));
	WRITE32(smram_state + SMRAM_ESI, REG32(ESI));
	WRITE32(smram_state + SMRAM_EDI, REG32(EDI));
	WRITE32(smram_state + SMRAM_EIP, m_eip);
	WRITE32(smram_state + SMRAM_EFLAGS, old_flags);
	WRITE32(smram_state + SMRAM_CR3, m_cr[3]);
	WRITE32(smram_state + SMRAM_CR0, old_cr0);

	m_sreg[DS].selector = m_sreg[ES].selector = m_sreg[FS].selector = m_sreg[GS].selector = m_sreg[SS].selector = 0;
	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffffffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x8093;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid = true;
	m_sreg[CS].selector = 0x3000; // pentium only, ppro sel = smbase >> 4
	m_sreg[CS].base = m_smbase;
	m_sreg[CS].limit = 0xffffffff;
	m_sreg[CS].flags = 0x8093;
	m_sreg[CS].valid = true;
	m_cr[4] = 0;
	m_dr[7] = 0x400;
	m_eip = 0x8000;

	m_nmi_masked = true;
	CHANGE_PC(m_eip);
}

void i386_device::leave_smm()
{
	uint32_t smram_state = m_smbase + 0xfe00;

	// load state, no sanity checks anywhere
	m_smbase = READ32(smram_state + SMRAM_SMBASE);
	m_cr[4] = READ32(smram_state + SMRAM_IP5_CR4);
	m_sreg[ES].limit = READ32(smram_state + SMRAM_IP5_ESLIM);
	m_sreg[ES].base = READ32(smram_state + SMRAM_IP5_ESBASE);
	m_sreg[ES].flags = READ32(smram_state + SMRAM_IP5_ESACC);
	m_sreg[CS].limit = READ32(smram_state + SMRAM_IP5_CSLIM);
	m_sreg[CS].base = READ32(smram_state + SMRAM_IP5_CSBASE);
	m_sreg[CS].flags = READ32(smram_state + SMRAM_IP5_CSACC);
	m_sreg[SS].limit = READ32(smram_state + SMRAM_IP5_SSLIM);
	m_sreg[SS].base = READ32(smram_state + SMRAM_IP5_SSBASE);
	m_sreg[SS].flags = READ32(smram_state + SMRAM_IP5_SSACC);
	m_sreg[DS].limit = READ32(smram_state + SMRAM_IP5_DSLIM);
	m_sreg[DS].base = READ32(smram_state + SMRAM_IP5_DSBASE);
	m_sreg[DS].flags = READ32(smram_state + SMRAM_IP5_DSACC);
	m_sreg[FS].limit = READ32(smram_state + SMRAM_IP5_FSLIM);
	m_sreg[FS].base = READ32(smram_state + SMRAM_IP5_FSBASE);
	m_sreg[FS].flags = READ32(smram_state + SMRAM_IP5_FSACC);
	m_sreg[GS].limit = READ32(smram_state + SMRAM_IP5_GSLIM);
	m_sreg[GS].base = READ32(smram_state + SMRAM_IP5_GSBASE);
	m_sreg[GS].flags = READ32(smram_state + SMRAM_IP5_GSACC);
	m_ldtr.flags = READ32(smram_state + SMRAM_IP5_LDTACC);
	m_ldtr.limit = READ32(smram_state + SMRAM_IP5_LDTLIM);
	m_ldtr.base = READ32(smram_state + SMRAM_IP5_LDTBASE);
	m_gdtr.limit = READ32(smram_state + SMRAM_IP5_GDTLIM);
	m_gdtr.base = READ32(smram_state + SMRAM_IP5_GDTBASE);
	m_idtr.limit = READ32(smram_state + SMRAM_IP5_IDTLIM);
	m_idtr.base = READ32(smram_state + SMRAM_IP5_IDTBASE);
	m_task.limit = READ32(smram_state + SMRAM_IP5_TRLIM);
	m_task.base = READ32(smram_state + SMRAM_IP5_TRBASE);
	m_task.flags = READ32(smram_state + SMRAM_IP5_TRACC);

	m_sreg[ES].selector = READ32(smram_state + SMRAM_ES);
	m_sreg[CS].selector = READ32(smram_state + SMRAM_CS);
	m_sreg[SS].selector = READ32(smram_state + SMRAM_SS);
	m_sreg[DS].selector = READ32(smram_state + SMRAM_DS);
	m_sreg[FS].selector = READ32(smram_state + SMRAM_FS);
	m_sreg[GS].selector = READ32(smram_state + SMRAM_GS);
	m_ldtr.segment = READ32(smram_state + SMRAM_LDTR);
	m_task.segment = READ32(smram_state + SMRAM_TR);

	m_dr[7] = READ32(smram_state + SMRAM_DR7);
	m_dr[6] = READ32(smram_state + SMRAM_DR6);
	REG32(EAX) = READ32(smram_state + SMRAM_EAX);
	REG32(ECX) = READ32(smram_state + SMRAM_ECX);
	REG32(EDX) = READ32(smram_state + SMRAM_EDX);
	REG32(EBX) = READ32(smram_state + SMRAM_EBX);
	REG32(ESP) = READ32(smram_state + SMRAM_ESP);
	REG32(EBP) = READ32(smram_state + SMRAM_EBP);
	REG32(ESI) = READ32(smram_state + SMRAM_ESI);
	REG32(EDI) = READ32(smram_state + SMRAM_EDI);
	m_eip = READ32(smram_state + SMRAM_EIP);
	m_eflags = READ32(smram_state + SMRAM_EFLAGS);
	m_cr[3] = READ32(smram_state + SMRAM_CR3);
	m_cr[0] = READ32(smram_state + SMRAM_CR0);

	m_CPL = (m_sreg[SS].flags >> 13) & 3; // cpl == dpl of ss

	for (int i = 0; i <= GS; i++)
	{
		if (PROTECTED_MODE && !V8086_MODE)
		{
			m_sreg[i].valid = m_sreg[i].selector ? true : false;
			m_sreg[i].d = (m_sreg[i].flags & 0x4000) ? 1 : 0;
		}
		else
			m_sreg[i].valid = true;
	}

	if (!m_smiact.isnull())
		m_smiact(false);
	m_smm = false;

	CHANGE_PC(m_eip);
	m_nmi_masked = false;
}

void i386_device::execute_set_input(int irqline, int state)
{
	if ( irqline == INPUT_LINE_A20 )
	{
		i386_set_a20_line( state );
		return;
	}
	if ( irqline == INPUT_LINE_NMI )
	{
		if ( state != CLEAR_LINE && m_halted)
		{
			m_halted = 0;
		}

		/* NMI (I do not think that this is 100% right) */
		if(m_nmi_masked)
		{
			m_nmi_latched = true;
			return;
		}
		if ( state )
			i386_trap(2, 1, 0);
	}
	else
	{
		if (irqline >= 0 && irqline <= MAX_INPUT_LINES)
		{
			if ( state != CLEAR_LINE && m_halted )
			{
				m_halted = 0;
			}

			m_irq_state = state;
		}
	}
}

void pentium_device::execute_set_input(int irqline, int state)
{
	if ( irqline == INPUT_LINE_SMI )
	{
		if ( !m_smi && state && m_smm )
		{
			m_smi_latched = true;
		}
		m_smi = state;
	}
	else
	{
		i386_device::execute_set_input(irqline, state);
	}
}

void i386_device::i386_set_a20_line(int state)
{
	if (state)
	{
		m_a20_mask = ~0;
	}
	else
	{
		m_a20_mask = ~(1 << 20);
	}
	// TODO: how does A20M and the tlb interact
	vtlb_flush_dynamic();
}

void i386_device::execute_run()
{
	int cycles = m_cycles;
	m_base_cycles = cycles;
	CHANGE_PC(m_eip);

	if (m_halted)
	{
		m_tsc += cycles;
		m_cycles = 0;
		return;
	}

	while( m_cycles > 0 )
	{
		i386_check_irq_line();

		// The LE and GE bits of DR7 aren't currently implemented because they could potentially require cycle-accurate emulation.
		if((m_dr[7] & 0xff) != 0) // If all of the breakpoints are disabled, skip checking for instruction breakpoint hitting entirely.
		for(int i = 0; i < 4; i++)
		{
			bool dri_enabled = (m_dr[7] & (1 << ((i << 1) + 1))) || (m_dr[7] & (1 << (i << 1))); // Check both local AND global enable bits for this breakpoint.
			if(dri_enabled && !m_RF)
			{
				int breakpoint_type = (m_dr[7] >> (i << 2)) & 3;
				int breakpoint_length = (m_dr[7] >> ((i << 2) + 2)) & 3;
				if(breakpoint_type == 0)
				{
					uint32_t phys_addr = 0;
					uint32_t error;
					phys_addr = (m_cr[0] & (1 << 31)) ? translate_address(m_CPL, TRANSLATE_FETCH, &m_dr[i], &error) : m_dr[i];
					if(breakpoint_length != 0) // Not one byte in length? logerror it, I have no idea how this works on real processors.
					{
						logerror("i386: Breakpoint length not 1 byte on an instruction breakpoint\n");
					}
					if(m_pc == phys_addr)
					{
						// The processor never automatically clears bits in DR6. It only sets them.
						m_dr[6] |= 1 << i;
						i386_trap(1,0,0);
						break;
					}
				}
			}
		}

		m_operand_size = m_sreg[CS].d;
		m_xmm_operand_size = 0;
		m_address_size = m_sreg[CS].d;
		m_operand_prefix = 0;
		m_address_prefix = 0;

		m_ext = 1;
		int old_tf = m_TF;

		m_segment_prefix = 0;
		m_prev_eip = m_eip;

		debugger_instruction_hook(m_pc);

		if(m_delayed_interrupt_enable != 0)
		{
			m_IF = 1;
			m_delayed_interrupt_enable = 0;
		}
#ifdef DEBUG_MISSING_OPCODE
		m_opcode_bytes_length = 0;
		m_opcode_pc = m_pc;
		m_opcode_addrs[m_opcode_addrs_index] = m_opcode_pc;
		m_opcode_addrs_index = (m_opcode_addrs_index + 1) & 15;
#endif
		try
		{
			i386_decode_opcode();
			if(m_TF && old_tf)
			{
				m_prev_eip = m_eip;
				m_ext = 1;
				m_dr[6] |= (1 << 14); //Set BS bit of DR6.
				i386_trap(1,0,0);
			}
			if(m_lock && (m_opcode != 0xf0))
				m_lock = false;
		}
		catch(uint64_t e)
		{
			m_ext = 1;
			i386_trap_with_error(e&0xffffffff,0,0,e>>32);
		}
		if(m_RF && m_auto_clear_RF) m_RF = 0;
		if(!m_auto_clear_RF) m_auto_clear_RF = true;
	}
	m_tsc += (cycles - m_cycles);
}

/*************************************************************************/

bool i386_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	bool ret = true;
	if(spacenum == AS_PROGRAM)
		ret = i386_translate_address(intention, &address, nullptr);
	address &= m_a20_mask;
	return ret;
}

int i386_device::get_mode() const
{
	return m_sreg[CS].d ? 32 : 16;
}

std::unique_ptr<util::disasm_interface> i386_device::create_disassembler()
{
	return std::make_unique<i386_disassembler>(this);
}

void i386_device::opcode_cpuid()
{
	logerror("CPUID called with unsupported EAX=%08x at %08x!\n", REG32(EAX), m_eip);
}

uint64_t i386_device::opcode_rdmsr(bool &valid_msr)
{
	valid_msr = false;
	logerror("RDMSR called with unsupported ECX=%08x at %08x!\n", REG32(ECX), m_eip);
	return -1;
}

void i386_device::opcode_wrmsr(uint64_t data, bool &valid_msr)
{
	valid_msr = false;
	logerror("WRMSR called with unsupported ECX=%08x (%08x%08x) at %08x!\n", REG32(ECX), (uint32_t)(data >> 32), (uint32_t)data, m_eip);
}

/*****************************************************************************/
/* Intel 486 */


void i486_device::device_start()
{
	i386_common_init();

	build_opcode_table(OP_I386 | OP_FPU | OP_I486);
	build_x87_opcode_table();
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_I486].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_I486].get();

	register_state_i386_x87();
}

void i486_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0;
	m_eflags_mask = 0x00077fd7;
	m_eip = 0xfff0;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4 (486), Model 0/1 (DX), Stepping 3
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (0 << 4) | (3);
	m_cpu_version = REG32(EDX);

	CHANGE_PC(m_eip);
}

void i486dx4_device::device_reset()
{
	i486_device::device_reset();
	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);
}

/*****************************************************************************/
/* Pentium */


void pentium_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM);
	build_x87_opcode_table();
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();
}

void pentium_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x003f7fd7;
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
	// Family 5 (Pentium), Model 2 (75 - 200MHz), Stepping 5
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (2 << 4) | (5);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	m_feature_flags = 0x000001bf;

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Cyrix MediaGX */


void mediagx_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_CYRIX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_MEDIAGX].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_MEDIAGX].get();
}

void mediagx_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4, Model 4 (MediaGX)
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (4 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x69727943;   // Cyri
	m_cpuid_id1 = 0x736e4978;   // xIns
	m_cpuid_id2 = 0x6d616574;   // tead

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x00000001;

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium Pro */

void pentium_pro_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium_pro_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

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
	// Family 6, Model 1 (Pentium Pro)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (1 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [15:15] CMOV and FCMOV
	// No MMX
	m_feature_flags = 0x000081bf;

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Intel Pentium MMX */

void pentium_mmx_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_MMX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium_mmx_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

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
	// Family 5, Model 4 (P55C)
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (4 << 4) | (1);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [23:23] MMX instructions
	m_feature_flags = 0x008001bf;

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium II */

void pentium2_device::device_start()
{
	i386_common_init();
	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium2_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

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
	// Family 6, Model 3 (Pentium II / Klamath)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (3 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x008081bf;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium III */

void pentium3_device::device_start()
{
	i386_common_init();
	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium3_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

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
	// Family 6, Model 8 (Pentium III / Coppermine)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (8 << 4) | (10);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x03;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 4:4] Time Stamp Counter
	// [ D:D] PTE Global Bit
	m_feature_flags = 0x00002011;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium 4 */

void pentium4_device::device_start()
{
	i386_common_init();
	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE | OP_SSE2);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium4_device::device_reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

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

	// [27:20] Extended family
	// [19:16] Extended model
	// [13:12] Type
	// [11: 8] Family
	// [ 7: 4] Model
	// [ 3: 0] Stepping ID
	// Family 15, Model 0 (Pentium 4 / Willamette)
	REG32(EAX) = 0;
	REG32(EDX) = (0 << 20) | (0xf << 8) | (0 << 4) | (1);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x00000001;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}
