// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawasaki Steel (Kawatetsu) KC82 CPU core with MMU

***************************************************************************/

#include "emu.h"
#include "kc82.h"

#define VERBOSE 0
#include "logmacro.h"


//-------------------------------------------------
//  kc82_device - constructor
//-------------------------------------------------

kc82_device::kc82_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor io_map)
	: z80_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, 16, 10, mem_map)
	, m_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 20, 0, 16, 10, mem_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0, io_map)
{
	std::fill_n(&m_mmu_a[0], 4, 0);
	std::fill_n(&m_mmu_b[0], 5, 0);
	std::fill_n(&m_mmu_base[0], 0x40, 0);
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector kc82_device::memory_space_config() const
{
	if (has_space(AS_OPCODES))
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config),
			std::make_pair(AS_OPCODES, &m_opcodes_config)
		};
	}
	else
	{
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc82_device::device_start()
{
	z80_device::device_start();

	for (int n = 1; n <= 4; n++)
	{
		state_add(KC82_B1 + n - 1, string_format("B%d", n).c_str(), m_mmu_b[n],
			[this, n](u8 data) { m_mmu_b[n] = data; mmu_remap_pages(); }
		).mask(0x3f);
		if (n != 4)
			state_add(KC82_A1 + n - 1, string_format("A%d", n).c_str(), m_mmu_a[n],
				[this, n](u16 data) { m_mmu_a[n] = data; mmu_remap_pages(); }
			).mask(0x3ff);
	}

	save_item(NAME(m_mmu_a));
	save_item(NAME(m_mmu_b));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc82_device::device_reset()
{
	z80_device::device_reset();

	std::fill_n(&m_mmu_a[1], 3, 0);
	std::fill_n(&m_mmu_b[1], 4, 0x3f);
	std::fill_n(&m_mmu_base[0], 0x40, 0);
}


//-------------------------------------------------
//  device_post_load - called after loading a
//  saved state
//-------------------------------------------------

void kc82_device::device_post_load()
{
	z80_device::device_post_load();

	mmu_remap_pages();
}


//**************************************************************************
//  KC82 MMU
//**************************************************************************

//-------------------------------------------------
//  mmu_remap_pages - recalculate base addresses
//  for memory pages
//-------------------------------------------------

void kc82_device::mmu_remap_pages()
{
	int n = 4;
	u32 base = 0xf0000; // A4 is fixed
	for (u8 i = 0x3f; i != 0; --i)
	{
		while (n != 0 && m_mmu_b[n] >= i)
		{
			--n;
			base = u32(m_mmu_a[n]) << 10;
		}
		if (m_mmu_base[i] != base)
		{
			u32 old_mapping = ((i << 10) + base) & 0xffc00;
			u32 new_mapping = ((i << 10) + m_mmu_base[i]) & 0xffc00;
			LOG("%s: MMU: %04X-%04XH => %05X-%05XH (was %05X-%05XH)\n",
				machine().describe_context(),
				i << 10, (i << 10) | 0x3ff,
				old_mapping, old_mapping | 0x3ff,
				new_mapping, new_mapping | 0x3ff);
		}
		m_mmu_base[i] = base;
	}
}


//-------------------------------------------------
//  mmu_r - read MMU register
//-------------------------------------------------

u8 kc82_device::mmu_r(offs_t offset)
{
	int n = (offset >> 1) + 1;
	if (BIT(offset, 0))
	{
		// read base register BRn
		return n == 4 ? 0xf0 : (m_mmu_a[n] & 0x3fc) >> 2;
	}
	else
	{
		// read boundary/base register BBRn
		return (n == 4 ? 0 : (m_mmu_a[n] & 0x003) << 6) | m_mmu_b[n];
	}
}


//-------------------------------------------------
//  mmu_w - write to MMU register
//-------------------------------------------------

void kc82_device::mmu_w(offs_t offset, u8 data)
{
	int n = (offset >> 1) + 1;
	if (BIT(offset, 0))
	{
		// write to base register BRn
		if (n != 4)
			m_mmu_a[n] = u16(data) << 2 | (m_mmu_a[n] & 0x003);
		else if (data != 0xf0)
			logerror("%s: Attempt to write %02X to upper 8 bits of A4\n", machine().describe_context(), data);
	}
	else
	{
		// write to boundary/base register BBRn
		m_mmu_b[n] = data & 0x3f;
		if (n != 4)
			m_mmu_a[n] = (m_mmu_a[n] & 0x3fc) | (data & 0xc0) >> 6;
		else if ((data & 0xc0) != 0)
			logerror("%s: Attempt to write %d to lower 2 bits of A4\n", machine().describe_context(), (data & 0xc0) >> 6);
	}
	mmu_remap_pages();
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  physical addresses
//-------------------------------------------------

bool kc82_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	if (spacenum == AS_PROGRAM || spacenum == AS_OPCODES)
		address = (address + m_mmu_base[(address & 0xfc00) >> 10]) & 0xfffff;
	target_space = &space(spacenum);
	return true;
}


//-------------------------------------------------
//  rm - read one byte from memory
//-------------------------------------------------

u8 kc82_device::data_read(u16 addr)
{
	return m_data.read_byte(addr + m_mmu_base[addr >> 10]);
}


//-------------------------------------------------
//  wm - write one byte to memory
//-------------------------------------------------

void kc82_device::data_write(u16 addr, u8 value)
{
	m_data.write_byte(addr + m_mmu_base[addr >> 10], value);
}


//-------------------------------------------------
//  rop - read opcode
//-------------------------------------------------

u8 kc82_device::opcode_read()
{
	u32 pc = m_pc.w + m_mmu_base[m_pc.b.h >> 2];
	// no refresh
	return m_opcodes.read_byte(pc);
}


//-------------------------------------------------
//  arg - read 8-bit argument
//-------------------------------------------------

u8 kc82_device::arg_read()
{
	u32 pc = m_pc.w + m_mmu_base[m_pc.b.h >> 2];
	return m_args.read_byte(pc);
}
