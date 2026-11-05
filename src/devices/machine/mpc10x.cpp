// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    Shared Motorola MPC105/MPC106 PCI host bridge base device
    Emulation by R. Belmont

    The MPC105 and MPC106 are very similar: map A (PREP) is the same on
    both, but map B is CHRP on the MPC106 and the 105's is different.
    There are a few other changes as well.

**********************************************************************/
#include "emu.h"
#include "mpc10x.h"

#include <algorithm>

#define LOG_RAM     (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

mpc10x_host_device::mpc10x_host_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pci_host_device(mconfig, type, tag, owner, clock)
	, m_ram_size(0)
	, m_map_type(MAP_TYPE_A)
	, m_cpu_space(nullptr)
	, m_pwrconfig1(0)
	, m_memory_starts{ 0, 0, 0, 0 }
	, m_memory_ends{ 0, 0, 0, 0 }
	, m_picr1(0)
	, m_memory_bank_enable(0)
	, m_mem_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_secondary_cpu(*this, finder_base::DUMMY_TAG)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_ram(nullptr)
{
}

void mpc10x_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x70, 0x71).rw(FUNC(mpc10x_host_device::pwrconfig1_r), FUNC(mpc10x_host_device::pwrconfig1_w));
	map(0x80, 0x8f).rw(FUNC(mpc10x_host_device::memory_start_r), FUNC(mpc10x_host_device::memory_start_w));
	map(0x90, 0x9f).rw(FUNC(mpc10x_host_device::memory_end_r), FUNC(mpc10x_host_device::memory_end_w));
	map(0xa0, 0xa0).rw(FUNC(mpc10x_host_device::memory_enable_r), FUNC(mpc10x_host_device::memory_enable_w));
}

void mpc10x_host_device::set_ram_info(u8 *ram_ptr, int ram_size)
{
	m_ram = ram_ptr;
	m_ram_size = ram_size;
}

void mpc10x_host_device::set_rom_tag(const char *tag)
{
	if (tag)
	{
		m_rom.set_tag(tag);
	}
}

void mpc10x_host_device::set_map_type(map_type maptype)
{
	m_map_type = maptype;
}

void mpc10x_host_device::device_start()
{
	pci_host_device::device_start();
	m_cpu_space = &m_cpu->space(AS_PROGRAM);
	m_cpu_spaces.push_back(m_cpu_space);
	if (m_secondary_cpu)
	{
		m_cpu_spaces.push_back(&m_secondary_cpu->space(AS_PROGRAM));
	}
	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));
	space(AS_PCI_IO).unmap_value_high();

	memory_window_start = 0;
	memory_window_end = 0xffff'ffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffff'ffff;
	io_offset = 0;
	command = 0x0006;
	status = 0x0080;

	// boards with a split ROM/flash decode map their own ROM
	if (m_rom.found())
	{
		const u64 rom_base = 0x1'0000'0000ULL - m_rom->bytes();
		for (address_space *space : m_cpu_spaces)
		{
			space->install_rom(rom_base, 0xffff'ffff, m_rom->base());
		}
	}

	for (address_space *space : m_cpu_spaces)
	{
		install_cpu_map(*space);
	}

	save_item(NAME(m_pwrconfig1));
	save_item(NAME(m_memory_starts));
	save_item(NAME(m_memory_ends));
	save_item(NAME(m_memory_bank_enable));
	save_item(NAME(m_picr1));
}

device_memory_interface::space_config_vector mpc10x_host_device::memory_space_config() const
{
	auto r = pci_bridge_device::memory_space_config();
	r.emplace_back(std::make_pair(AS_PCI_MEM, &m_mem_config));
	r.emplace_back(std::make_pair(AS_PCI_IO, &m_io_config));
	return r;
}

// The RAM banks and the CONFIG_ADDR/CONFIG_DATA window live in the CPU's space and are only
// installed when the firmware programs them, so put them back after a state load.
void mpc10x_host_device::device_post_load()
{
	install_ram();
	update_config_access();
}

void mpc10x_host_device::update_config_access()
{
	for (address_space *space : m_cpu_spaces)
	{
		install_config_access_map(*space);
	}
}

u32 mpc10x_host_device::be_config_address_r()
{
	return swapendian_int32(pci_host_device::config_address_r());
}

void mpc10x_host_device::be_config_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	pci_host_device::config_address_w(offset, swapendian_int32(data), swapendian_int32(mem_mask));
}

u32 mpc10x_host_device::be_config_data_r(offs_t offset, u32 mem_mask)
{
	return swapendian_int32(pci_host_device::config_data_r(offset, swapendian_int32(mem_mask)));
}

void mpc10x_host_device::be_config_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	pci_host_device::config_data_w(offset, swapendian_int32(data), swapendian_int32(mem_mask));
}

template <u32 Base>
u32 mpc10x_host_device::cpu_memory_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		return m_cpu_space->read_dword(Base + ((offset * 4) ^ 4), mem_mask);
	}
	else
	{
		return swapendian_int32(m_cpu_space->read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
	}
}

template <u32 Base>
void mpc10x_host_device::cpu_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		m_cpu_space->write_dword(Base + ((offset * 4) ^ 4), data, mem_mask);
	}
	else
	{
		m_cpu_space->write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32(mem_mask));
	}
}

template u32 mpc10x_host_device::cpu_memory_r<0xff00'0000>(offs_t offset, u32 mem_mask);
template u32 mpc10x_host_device::cpu_memory_r<0>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::cpu_memory_w<0>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 mpc10x_host_device::pci_memory_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		return this->space(AS_PCI_MEM).read_dword(Base + ((offset * 4) ^ 4), mem_mask);
	}
	else
	{
		return swapendian_int32(this->space(AS_PCI_MEM).read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
	}
}

template <u32 Base>
void mpc10x_host_device::pci_memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		this->space(AS_PCI_MEM).write_dword(Base + ((offset * 4) ^ 4), data, mem_mask);
	}
	else
	{
		this->space(AS_PCI_MEM).write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32(mem_mask));
	}
}

template u32 mpc10x_host_device::pci_memory_r<0xf100'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_memory_w<0xf100'0000>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_memory_r<0xf900'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_memory_w<0xf900'0000>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_memory_r<0>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_memory_w<0>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_memory_r<0x8000'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_memory_w<0x8000'0000>(offs_t offset, u32 data, u32 mem_mask);

template <u32 Base>
u32 mpc10x_host_device::pci_io_r(offs_t offset, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		return this->space(AS_PCI_IO).read_dword(Base + ((offset * 4) ^ 4), mem_mask);
	}
	else
	{
		return swapendian_int32(this->space(AS_PCI_IO).read_dword(Base + (offset * 4), swapendian_int32(mem_mask)));
	}
}

template <u32 Base>
void mpc10x_host_device::pci_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_picr1 & PICR1_LE_MODE)
	{
		this->space(AS_PCI_IO).write_dword(Base + ((offset * 4) ^ 4), data, mem_mask);
	}
	else
	{
		this->space(AS_PCI_IO).write_dword(Base + (offset * 4), swapendian_int32(data), swapendian_int32(mem_mask));
	}
}

template u32 mpc10x_host_device::pci_io_r<0xf000'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_io_w<0xf000'0000>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_io_r<0xf800'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_io_w<0xf800'0000>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_io_r<0>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_io_w<0>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_io_r<0x0100'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_io_w<0x0100'0000>(offs_t offset, u32 data, u32 mem_mask);
template u32 mpc10x_host_device::pci_io_r<0x0080'0000>(offs_t offset, u32 mem_mask);
template void mpc10x_host_device::pci_io_w<0x0080'0000>(offs_t offset, u32 data, u32 mem_mask);

u16 mpc10x_host_device::pwrconfig1_r()
{
	return m_pwrconfig1;
}

void mpc10x_host_device::pwrconfig1_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pwrconfig1);
}

u32 mpc10x_host_device::memory_start_r(offs_t offset)
{
	return m_memory_starts[offset];
}

void mpc10x_host_device::memory_start_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memory_starts[offset]);
	LOGMASKED(LOG_RAM, "%s: %08x to memory start @ %x\n", machine().describe_context(), data, offset);
}

u32 mpc10x_host_device::memory_end_r(offs_t offset)
{
	return m_memory_ends[offset];
}

void mpc10x_host_device::memory_end_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_memory_ends[offset]);
	LOGMASKED(LOG_RAM, "%s: %08x to memory end @ %x\n", machine().describe_context(), data, offset);
}

u8 mpc10x_host_device::memory_enable_r()
{
	return m_memory_bank_enable;
}

void mpc10x_host_device::memory_enable_w(u8 data)
{
	LOGMASKED(LOG_RAM, "%s: %02x to memory_enable\n", machine().describe_context(), data);
	m_memory_bank_enable = data;
	install_ram();
}

void mpc10x_host_device::install_ram()
{
	// unmap all RAM
	for (address_space *space : m_cpu_spaces)
	{
		space->unmap_readwrite(0x0000'0000, 0x0fff'ffff);
	}
	install_memory_banks(true, true);
}

void mpc10x_host_device::install_memory_banks(bool skip_ff, bool wrap_banks)
{
	u64 base = m_memory_starts[0] | (u64(m_memory_starts[1]) << 32);
	u64 base2 = m_memory_starts[2] | (u64(m_memory_starts[3]) << 32);
	u64 end = m_memory_ends[0] | (u64(m_memory_ends[1]) << 32);
	u64 end2 = m_memory_ends[2] | (u64(m_memory_ends[3]) << 32);
	u32 bank_start = 0;
	u32 bank_end = 0;
	u64 install_ptr = 0;

	for (int bank = 0; bank < 8; bank++)
	{
		if ((!skip_ff || ((base & 0xff) != 0xff)) && BIT(m_memory_bank_enable, bank))
		{
			bank_start = (base & 0xff) << 20;
			bank_start |= (base2 & 0xff) << 28;
			bank_end = (end & 0xff) << 20;
			bank_end |= (end2 & 0xff) << 28;
			bank_end |= 0xfffff;

			if ((bank_end >= bank_start) && (m_ram_size > 0) && (wrap_banks || (install_ptr < u64(m_ram_size))))
			{
				const u64 bank_size = u64(bank_end - bank_start) + 1;

				LOGMASKED(LOG_RAM, "bank %d: start %08x end %08x size %llx, install_ptr = %llx\n", bank, bank_start, bank_end, bank_size, install_ptr);

				// banks larger than the installed RAM mirror it (the iMac ROM relies on this)
				u64 ptr = install_ptr % u64(m_ram_size);
				u32 start = bank_start;
				u64 remaining = bank_size;
				while (remaining > 0)
				{
					const u64 chunk = std::min<u64>(remaining, u64(m_ram_size) - ptr);
					for (address_space *space : m_cpu_spaces)
					{
						space->install_ram(start, start + chunk - 1, &m_ram[ptr]);
					}
					start += chunk;
					remaining -= chunk;
					ptr = 0;
				}
				install_ptr += bank_size;
			}
		}

		base >>= 8;
		base2 >>= 8;
		end >>= 8;
		end2 >>= 8;
	}
}

u32 mpc10x_host_device::picr1_r()
{
	return m_picr1;
}

