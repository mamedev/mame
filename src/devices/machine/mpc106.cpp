// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mpc106.cpp - Motorola MPC106 PCI host bridge, aka "Grackle".

    Can support up to 4 PowerPC CPUs, up to 1 GiB of RAM, and 16 MiB of ROM

**********************************************************************/
#include "emu.h"
#include "mpc106.h"

#define LOG_RAM     (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

enum
{
	AS_PCI_MEM = 1,
	AS_PCI_IO = 2
};

constexpr u32 PICR1_LE_MODE = 0x00000020;   // Host CPU is little-endian if set, big-endian if clear

DEFINE_DEVICE_TYPE(MPC106, mpc106_host_device, "mpc106", "Motorola MPC106 PCI Bridge/Memory Controller")

void mpc106_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x70, 0x71).rw(FUNC(mpc106_host_device::pwrconfig1_r), FUNC(mpc106_host_device::pwrconfig1_w));
	map(0x72, 0x72).rw(FUNC(mpc106_host_device::pwrconfig2_r), FUNC(mpc106_host_device::pwrconfig2_w));
	map(0x80, 0x8f).rw(FUNC(mpc106_host_device::memory_start_r), FUNC(mpc106_host_device::memory_start_w));
	map(0x90, 0x9f).rw(FUNC(mpc106_host_device::memory_end_r), FUNC(mpc106_host_device::memory_end_w));
	map(0xa0, 0xa0).rw(FUNC(mpc106_host_device::memory_enable_r), FUNC(mpc106_host_device::memory_enable_w));
	map(0xa8, 0xab).rw(FUNC(mpc106_host_device::picr1_r), FUNC(mpc106_host_device::picr1_w));
}

mpc106_host_device::mpc106_host_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_host_device(mconfig, MPC106, tag, owner, clock)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_cpu(*this, finder_base::DUMMY_TAG)
{
	set_ids_host(0x10570002, 0x40, 0x000006);
	m_memory_bank_enable = 0;
	m_picr1 = 0;
}

void mpc106_host_device::set_ram_info(u8 *ram_ptr, int ram_size)
{
	m_ram = ram_ptr;
	m_ram_size = ram_size;
}

void mpc106_host_device::set_rom_tag(const char *tag)
{
	m_rom_tag = tag;
}

void mpc106_host_device::set_map_type(map_type maptype)
{
	m_map_type = maptype;
}

void mpc106_host_device::device_start()
{
	pci_host_device::device_start();
	m_cpu_space = &m_cpu->space(AS_PCI_CONFIG);
	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;
	command = 0x0006;
	status = 0x0080;
	revision = 0x40;

	m_rom = device().machine().root_device().memregion(m_rom_tag)->base();
	m_rom_size = device().machine().root_device().memregion(m_rom_tag)->bytes();

	m_pwrconfig1 = m_pwrconfig2 = 0;

	u64 rom_base = 0x100000000ULL - m_rom_size;
	m_cpu_space->install_rom(rom_base, 0xffffffff, m_rom);

	if (m_map_type == MAP_TYPE_A)
	{   // type A is PReP
		m_cpu_space->install_read_handler (0x80000000, 0x807fffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0>)));
		m_cpu_space->install_write_handler(0x80000000, 0x807fffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0>)));

		m_cpu_space->install_device(0x80000000, 0x80000cff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

		m_cpu_space->install_read_handler (0x81000000, 0xbf7fffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0x01000000>)));
		m_cpu_space->install_write_handler(0x81000000, 0xbf7fffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0x01000000>)));
		m_cpu_space->install_read_handler (0xc0000000, 0xfeffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0>)));
		m_cpu_space->install_write_handler(0xc0000000, 0xfeffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0>)));
	}
	else if (m_map_type == MAP_TYPE_B)
	{   // type B is CHRP
		m_cpu_space->install_read_handler (0x80000000, 0xfcffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0x80000000>)));
		m_cpu_space->install_write_handler(0x80000000, 0xfcffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0x80000000>)));
		m_cpu_space->install_read_handler (0xfd000000, 0xfdffffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_r<0>)));
		m_cpu_space->install_write_handler(0xfd000000, 0xfdffffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_memory_w<0>)));
		m_cpu_space->install_read_handler (0xfe000000, 0xfe00ffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0>)));
		m_cpu_space->install_write_handler(0xfe000000, 0xfe00ffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0>)));
		m_cpu_space->install_read_handler (0xfe800000, 0xfebfffff, read32s_delegate(*this, FUNC(mpc106_host_device::pci_io_r<0x00800000>)));
		m_cpu_space->install_write_handler(0xfe800000, 0xfebfffff, write32s_delegate(*this, FUNC(mpc106_host_device::pci_io_w<0x00800000>)));

		if (m_picr1 & PICR1_LE_MODE)
		{
			m_cpu_space->install_device(0xfec00000, 0xfeefffff, *static_cast<mpc106_host_device *>(this), &mpc106_host_device::access_map_le);
		}
		else
		{
			m_cpu_space->install_device(0xfec00000, 0xfeefffff, *static_cast<mpc106_host_device *>(this), &mpc106_host_device::access_map_be);
		}
	}

	save_item(NAME(m_pwrconfig1));
	save_item(NAME(m_pwrconfig2));
	save_item(NAME(m_memory_starts));
	save_item(NAME(m_memory_ends));
	save_item(NAME(m_memory_bank_enable));
	save_item(NAME(m_picr1));
}

device_memory_interface::space_config_vector mpc106_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

void mpc106_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void mpc106_host_device::device_reset()
{
	pci_host_device::device_reset();
}

void mpc106_host_device::access_map_le(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(mpc106_host_device::config_address_r), FUNC(mpc106_host_device::config_address_w));
	map(0x00200000, 0x002fffff).rw(FUNC(mpc106_host_device::config_data_r), FUNC(mpc106_host_device::config_data_w));
}

void mpc106_host_device::access_map_be(address_map &map)
{
	map(0x00000000, 0x001fffff).rw(FUNC(mpc106_host_device::be_config_address_r), FUNC(mpc106_host_device::be_config_address_w));
	map(0x00200000, 0x002fffff).rw(FUNC(mpc106_host_device::be_config_data_r), FUNC(mpc106_host_device::be_config_data_w));
}

u32 mpc106_host_device::be_config_address_r()
{
	return swapendian_int32(pci_host_device::config_address_r());
}

void mpc106_host_device::be_config_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	pci_host_device::config_address_w(offset, swapendian_int32(data), swapendian_int32(mem_mask));
}

u32 mpc106_host_device::be_config_data_r(offs_t offset, u32 mem_mask)
{
	return swapendian_int32(pci_host_device::config_data_r(offset, swapendian_int32(mem_mask)));
}

void mpc106_host_device::be_config_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	pci_host_device::config_data_w(offset, swapendian_int32(data), swapendian_int32(mem_mask));
}

template <u32 Base>
u32 mpc106_host_device::cpu_memory_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		return m_cpu_space->read_dword(Base + (offset * 4), mem_mask);
	}
	else
	{
		return swapendian_int32(m_cpu_space->read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
	}
}

template <u32 Base>
void mpc106_host_device::cpu_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		m_cpu_space->write_dword(Base + (offset * 4), data, mem_mask);
	}
	else
	{
		m_cpu_space->write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32(mem_mask));
	}
}

template u32 mpc106_host_device::cpu_memory_r<0>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::cpu_memory_r<0xff800000>(offs_t offset, u32 mem_mask);

template void mpc106_host_device::cpu_memory_w<0>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::cpu_memory_w<0xff800000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 mpc106_host_device::pci_memory_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		return this->space(AS_PCI_MEM).read_dword(Base + (offset * 4), mem_mask);
	}
	else
	{
		return swapendian_int32(this->space(AS_PCI_MEM).read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
	}
}

template <u32 Base>
void mpc106_host_device::pci_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		this->space(AS_PCI_MEM).write_dword(Base + (offset * 4), data, mem_mask);
	}
	else
	{
		this->space(AS_PCI_MEM).write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32((mem_mask)));
	}
}

template u32 mpc106_host_device::pci_memory_r<0>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::pci_memory_r<0x80000000>(offs_t offset, u32 mem_mask);

template void mpc106_host_device::pci_memory_w<0>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::pci_memory_w<0x80000000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 mpc106_host_device::pci_io_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		return this->space(AS_PCI_IO).read_dword(Base + (offset * 4), mem_mask);
	}
	else
	{
		return swapendian_int32(this->space(AS_PCI_IO).read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
	}
}

template <u32 Base>
void mpc106_host_device::pci_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		this->space(AS_PCI_IO).write_dword(Base + (offset * 4), data, mem_mask);
	}
	else
	{
		this->space(AS_PCI_IO).write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32((mem_mask)));
	}
}

template u32 mpc106_host_device::pci_io_r<0>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::pci_io_r<0x01000000>(offs_t offset, u32 mem_mask);
template u32 mpc106_host_device::pci_io_r<0x00800000>(offs_t offset, u32 mem_mask);

template void mpc106_host_device::pci_io_w<0>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::pci_io_w<0x01000000>(offs_t offset, u32 data, u32 mem_mask);
template void mpc106_host_device::pci_io_w<0x00800000>(offs_t offset, u32 data, u32 mem_mask);

// map PCI memory and I/O space stuff here
void mpc106_host_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
									 u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	// map RAM and ROM into the PCI memory space (because the PPC bus is 64 bits wide, and weirdness happens on DMA otherwise)
	memory_space->install_read_handler(0xff800000, 0xffffffff, read32s_delegate(*this, FUNC(mpc106_host_device::cpu_memory_r<0xff800000>)));

	memory_space->install_read_handler(0, m_ram_size - 1, read32s_delegate(*this, FUNC(mpc106_host_device::cpu_memory_r<0>)));
	memory_space->install_write_handler(0, m_ram_size - 1, write32s_delegate(*this, FUNC(mpc106_host_device::cpu_memory_w<0>)));
}

u16 mpc106_host_device::pwrconfig1_r()
{
	return m_pwrconfig1;
}

void mpc106_host_device::pwrconfig1_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pwrconfig1);
}

u8 mpc106_host_device::pwrconfig2_r()
{
		return m_pwrconfig2;
}

void mpc106_host_device::pwrconfig2_w(offs_t offset, u8 data)
{
	m_pwrconfig2 = data;
}

u32 mpc106_host_device::memory_start_r(offs_t offset)
{
	return m_memory_starts[offset];
}

void mpc106_host_device::memory_start_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memory_starts[offset]);
	LOGMASKED(LOG_RAM, "%s: %08x to memory start @ %x\n", tag(), data, offset);
}

u32 mpc106_host_device::memory_end_r(offs_t offset)
{
	return m_memory_ends[offset];
}

void mpc106_host_device::memory_end_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memory_ends[offset]);
	LOGMASKED(LOG_RAM, "%s: %08x to memory end @ %x\n", tag(), data, offset);
}

u8 mpc106_host_device::memory_enable_r()
{
	return m_memory_bank_enable;
}

void mpc106_host_device::memory_enable_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_RAM, "%s: %02x to memory_enable\n", tag(), data);
	m_memory_bank_enable = data;

	// unmap all RAM
	m_cpu_space->unmap_readwrite(0x00000000, 0x0fffffff);

	u64 base = m_memory_starts[0] | ((u64)m_memory_starts[1] << 32);
	u64 base2 = m_memory_starts[2] | ((u64)m_memory_starts[3] << 32);
	u64 end = m_memory_ends[0] | ((u64)m_memory_ends[0] << 32);
	u64 end2 = m_memory_ends[2] | ((u64)m_memory_ends[3] << 32);
	u32 bank_start = 0;
	u32 bank_end = 0;
	u64 install_ptr = 0;

	for (int bank = 0; bank < 8; bank++)
	{
		if (((base & 0xff) != 0xff) && (BIT(m_memory_bank_enable, bank)))
		{
			bank_start = (base & 0xff) << 20;
			bank_start |= (base2 & 0xff) << 28;
			bank_end = (end & 0xff) << 20;
			bank_end |= (end2 & 0xff) << 28;
			bank_end |= 0xfffff;

			LOGMASKED(LOG_RAM, "bank %d: start %08x end %08x, install_ptr = %llx\n", bank, bank_start, bank_end, install_ptr);

			m_cpu_space->install_ram(bank_start, bank_end, &m_ram[install_ptr]);
			install_ptr += (bank_end + 1);
		}

		base >>= 8;
		base2 >>= 8;
		end >>= 8;
		end2 >>= 8;
	}
}

u32 mpc106_host_device::picr1_r()
{
	return m_picr1;
}

void mpc106_host_device::picr1_w(u32 data)
{
	m_picr1 = data;
}
