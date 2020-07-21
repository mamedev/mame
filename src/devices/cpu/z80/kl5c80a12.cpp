// license:BSD-3-Clause
// copyright-holders:David Haywood,AJR
/***************************************************************************

    Kawasaki LSI
    KL5C80A12 CPU (KL5C80A12CFP on hng64.c)

    Binary compatible with Z80, significantly faster opcode timings, operating at up to 10Mhz
    Timers / Counters, Parallel / Serial ports/ MMU, Interrupt Controller

    (is this different enough to need it's own core?)
    (todo: everything except MMU)

***************************************************************************/

#include "emu.h"
#include "kl5c80a12.h"

DEFINE_DEVICE_TYPE(KL5C80A12, kl5c80a12_device, "kl5c80a12", "Kawasaki LSI KL5C80A12")


kl5c80a12_device::kl5c80a12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, KL5C80A12, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, 16, 10)
	, m_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 20, 0, 16, 10)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(kl5c80a12_device::internal_io), this))
{
	std::fill_n(&m_mmu_a[0], 4, 0);
	std::fill_n(&m_mmu_b[0], 5, 0);
	std::fill_n(&m_mmu_base[0], 0x40, 0);
}


//-------------------------------------------------
//  internal_io - map for internal I/O registers
//-------------------------------------------------

void kl5c80a12_device::internal_io(address_map &map)
{
	map(0x00, 0x07).mirror(0xff00).rw(FUNC(kl5c80a12_device::mmu_r), FUNC(kl5c80a12_device::mmu_w));
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector kl5c80a12_device::memory_space_config() const
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

void kl5c80a12_device::device_start()
{
	z80_device::device_start();

	for (int n = 1; n <= 4; n++)
	{
		state_add<u8>(KC82_B1 + n - 1, string_format("B%d", n).c_str(),
			[this, n]() { return m_mmu_b[n]; },
			[this, n](u8 data) { m_mmu_b[n] = data; mmu_remap_pages(); }
		).mask(0x3f);
		if (n != 4)
			state_add<u16>(KC82_A1 + n - 1, string_format("A%d", n).c_str(),
				[this, n]() { return m_mmu_a[n]; },
				[this, n](u16 data) { m_mmu_a[n] = data; mmu_remap_pages(); }
			).mask(0x3ff);
	}

	save_item(NAME(m_mmu_a));
	save_item(NAME(m_mmu_b));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kl5c80a12_device::device_reset()
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

void kl5c80a12_device::device_post_load()
{
	z80_device::device_post_load();

	mmu_remap_pages();
}


/* CPU interface */
void kl5c80a12_device::device_add_mconfig(machine_config &config)
{
}


//**************************************************************************
//  KC82 MMU
//**************************************************************************

//-------------------------------------------------
//  mmu_remap_pages - recalculate base addresses
//  for memory pages
//-------------------------------------------------

void kl5c80a12_device::mmu_remap_pages()
{
	int n = 4;
	u32 base = 0xf0000; // A4 is fixed
	for (int i = 0x3f; i >= 0; --i)
	{
		while (n != 0 && m_mmu_b[n] >= i)
		{
			--n;
			base = u32(m_mmu_a[n]) << 10;
		}
		m_mmu_base[i] = base;
	}
}


//-------------------------------------------------
//  mmu_r - read MMU register
//-------------------------------------------------

u8 kl5c80a12_device::mmu_r(offs_t offset)
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

void kl5c80a12_device::mmu_w(offs_t offset, u8 data)
{
	int n = (offset >> 1) + 1;
	if (BIT(offset, 0))
	{
		// write to base register BRn
		if (n != 4)
			m_mmu_a[n] = u16(data) << 2 | (m_mmu_a[n] & 0x003);
	}
	else
	{
		// write to boundary/base register BBRn
		m_mmu_b[n] = data & 0x3f;
		if (n != 4)
			m_mmu_a[n] = (m_mmu_a[n] & 0x3fc) | (data & 0xc0) >> 6;
	}
	mmu_remap_pages();
}


//-------------------------------------------------
//  memory_translate - translate from logical to
//  physical addresses
//-------------------------------------------------

bool kl5c80a12_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	if (spacenum == AS_PROGRAM || spacenum == AS_OPCODES)
		address = (address + m_mmu_base[(address & 0xfc00) >> 10]) & 0xfffff;
	return true;
}


//-------------------------------------------------
//  rm - read one byte from memory
//-------------------------------------------------

u8 kl5c80a12_device::rm(u16 addr)
{
	return m_data.read_byte(addr + m_mmu_base[addr >> 10]);
}


//-------------------------------------------------
//  wm - write one byte to memory
//-------------------------------------------------

void kl5c80a12_device::wm(u16 addr, u8 value)
{
	m_data.write_byte(addr + m_mmu_base[addr >> 10], value);
}


//-------------------------------------------------
//  rop - read opcode
//-------------------------------------------------

u8 kl5c80a12_device::rop()
{
	u32 pc = m_pc.w.l + m_mmu_base[m_pc.b.h >> 2];
	m_pc.w.l++;
	// no refresh
	return m_opcodes.read_byte(pc);
}


//-------------------------------------------------
//  arg - read 8-bit argument
//-------------------------------------------------

u8 kl5c80a12_device::arg()
{
	u32 pc = m_pc.w.l + m_mmu_base[m_pc.b.h >> 2];
	m_pc.w.l++;
	return m_args.read_byte(pc);
}


//-------------------------------------------------
//  arg16 - read 16-bit argument
//-------------------------------------------------

u16 kl5c80a12_device::arg16()
{
	u16 d16 = arg();
	d16 |= u16(arg()) << 8;
	return d16;
}
