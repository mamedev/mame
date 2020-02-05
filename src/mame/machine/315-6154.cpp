// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

#include "emu.h"
#include "machine/315-6154.h"

DEFINE_DEVICE_TYPE(SEGA315_6154, sega_315_6154_device, "sega315_6154", "Sega 315-6154 Northbridge")

sega_315_6154_device::sega_315_6154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEGA315_6154, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_memory_config("memory", ENDIANNESS_LITTLE, 32, 32),
	m_configuration_config("configuration", ENDIANNESS_LITTLE, 32, 32)
{
	memset(m_registers, 0, sizeof(m_registers));
	memset(m_bases, 0, sizeof(m_bases));
	m_useconfig_14x = false;
	m_useconfig_18x = false;
}

void sega_315_6154_device::device_start()
{
	m_memory = &space(AS_PCI_MEMORY);
	m_configuration = &space(AS_PCI_CONFIGURATION);

	save_item(NAME(m_registers));
	save_item(NAME(m_bases));
	save_item(NAME(m_useconfig_14x));
	save_item(NAME(m_useconfig_18x));
}

void sega_315_6154_device::device_reset()
{
	memset(m_registers, 0, sizeof(m_registers));
}

device_memory_interface::space_config_vector sega_315_6154_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PCI_MEMORY, &m_memory_config),
		std::make_pair(AS_PCI_CONFIGURATION, &m_configuration_config)
	};
}

READ32_MEMBER(sega_315_6154_device::registers_r)
{
	return m_registers[offset];
}

static inline void parse_address_register(u32 reg, u32 *base)
{
	base[0] = (reg & 0xff) << 24;
	reg = reg >> 8;
	base[1] = (reg & 0xff) << 24;
	reg = reg >> 8;
	base[2] = (reg & 0xff) << 24;
	reg = reg >> 8;
	base[3] = reg << 24;
}

WRITE32_MEMBER(sega_315_6154_device::registers_w)
{
	COMBINE_DATA(m_registers + offset);

	if (offset == 0x10 / 4)
		parse_address_register(m_registers[0x10 / 4], m_bases);
	else if (offset == 0x14 / 4)
		parse_address_register(m_registers[0x14 / 4], m_bases + 4);
	else if (offset == 0x18 / 4)
		parse_address_register(m_registers[0x18 / 4], m_bases + 8);
	else if (offset == 0x1c / 4)
	{
		m_useconfig_14x = (m_registers[0x1c / 4] & 1) != 0;
		m_useconfig_18x = (m_registers[0x1c / 4] & 2) != 0;
	}
	else if (offset == 0x38 / 4)
	{
		if (m_registers[0x38 / 4] & 0x01000000)
		{
			uint32_t s, d, l;

			s = m_registers[0x30 / 4];
			d = m_registers[0x34 / 4];
			l = m_registers[0x38 / 4] & 0xffffff;
			logerror("got dma transfer request from 0x%08x to 0x%08x size 0x%08x bytes\n", s, d, l << 2);
			while (l != 0)
			{
				m_memory->write_dword(d, m_memory->read_dword(s));
				s += 4;
				d += 4;
				l--;
			}
		}
		m_registers[0x38 / 4] &= ~0x01000000;
	}
}

template<int Aperture>
u32 sega_315_6154_device::aperture_r(address_space &space, offs_t offset, u32 mem_mask)
{
	const u32 destination_offset = offset & 0x3fffff;
	const int destination = (offset >> 22) & 3;
	const int index = Aperture * 4 + destination;

	if ((Aperture == 0) && (destination == 0) && (m_useconfig_14x == true))
		return m_configuration->read_dword(destination_offset << 2, mem_mask);
	if ((Aperture == 1) && (destination == 0) && (m_useconfig_18x == true))
		return m_configuration->read_dword(destination_offset << 2, mem_mask);
	return m_memory->read_dword(m_bases[index] + (destination_offset << 2), mem_mask);
}

template u32 sega_315_6154_device::aperture_r<0>(address_space &space, offs_t offset, u32 mem_mask);
template u32 sega_315_6154_device::aperture_r<1>(address_space &space, offs_t offset, u32 mem_mask);
template u32 sega_315_6154_device::aperture_r<2>(address_space &space, offs_t offset, u32 mem_mask);

template<int Aperture>
void sega_315_6154_device::aperture_w(address_space &space, offs_t offset, u32 data, u32 mem_mask)
{
	const u32 destination_offset = offset & 0x3fffff;
	const int destination = (offset >> 22) & 3;
	const int index = Aperture * 4 + destination;

	if ((Aperture == 0) && (destination == 0) && (m_useconfig_14x == true))
	{
		m_configuration->write_dword(destination_offset << 2, data, mem_mask);
		return;
	}
	if ((Aperture == 1) && (destination == 0) && (m_useconfig_18x == true))
	{
		m_configuration->write_dword(destination_offset << 2, data, mem_mask);
		return;
	}
	m_memory->write_dword(m_bases[index] + (destination_offset << 2), data, mem_mask);
}

template void sega_315_6154_device::aperture_w<0>(address_space &space, offs_t offset, u32 data, u32 mem_mask);
template void sega_315_6154_device::aperture_w<1>(address_space &space, offs_t offset, u32 data, u32 mem_mask);
template void sega_315_6154_device::aperture_w<2>(address_space &space, offs_t offset, u32 data, u32 mem_mask);
