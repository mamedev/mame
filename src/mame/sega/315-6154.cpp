// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli

/*
    Sega 315-6154 PCI bridge and bus controller for SH-4

    features:
     - x2 PCI interfaces "B" and "C", may be used for bridge-to-bridge linking
     - "local bus" - canonical data/address bus, have 29 dedicated address lines (e.g. addresses space below 0x40000000), reuses one of PCI interfaces as data bus.
     - SDRAM interface for local RAM
     - 64bit interface "A" to SH-4 CPU, uses MPX and DDT DMA mode, CS lines usage:
        /ACS0 - aperture 2
        /ACS1 - bridge control registers
        /ACS4 - bridge local SDRAM
        /ACS5 - aperture 0
        /ACS6 - aperture 1

    registers:
     00   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx bridge ID, 0 = main, otherwise slave
     04   ---M---- -------- ---Dmmmm mmssssss s/m - slave/main bridge bus errors, D - DMA done status (all prev bits cleared by writing 1), M - presumable DMA done interrupt mask
     10   33333333 22222222 11111111 00000000 Aperture 0 page 0/1/2/3 addresses(*)
     14   33333333 22222222 11111111 00000000 Aperture 1 page 0/1/2/3 addresses(*)
     18   33333333 22222222 11111111 00000000 Aperture 2 page 0/1/2/3 addresses(*)
     1C   -------- -------3 2222---- ------10 0/1 - switch aperture 0/1 to PCI Config space access mode, 2 - unknown (may be writen by DIMM), 3 - unknown always 1
     20   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx unknown configuration-related, always 0x00fe00fe
     24   hhhhgggg ffffeeee ddddcccc bbbbaaaa pages config(**), e - aperture 2 page 2, f - aperture 2 page 3, g/h - DMA source/destination, other is unknown
     28   -------- -------- -------- -------x unknown, 1 writen by DIMM firmware
     30   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx DMA source address(*)
     34   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx DMA destination address(*)
     38   -------s llllllll llllllll llllllll l - DMA length in 32bit units, s - DMA start/busy

     (*)  - meaning of top address bit(s) is not clear, possible if bit 31=1 and bit 30=0 this means "local bus access"
     (**) - function of config bits is unknown, value is 0-6 optionally |=8, top bit usually 1 for pages mapped to "local bus" area and 0 for PCI-mapped pages

    TODO:
     - bridge link
     - delay DMA done status/irq
     - figure out meaning of top bit in page address and DMA source/destination
     - figure out function of configuration registers 0x24 and 0x20
*/

#include "emu.h"
#include "315-6154.h"

DEFINE_DEVICE_TYPE(SEGA315_6154, sega_315_6154_device, "sega315_6154", "Sega 315-6154 Northbridge")

sega_315_6154_device::sega_315_6154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, SEGA315_6154, tag, owner, clock),
	m_configuration_config("configuration_space", ENDIANNESS_LITTLE, 32, 20),
	m_memory_config("memory_space", ENDIANNESS_LITTLE, 32, 32),
	irq1_cb(*this)
{
	memset(m_registers, 0, sizeof(m_registers));
	memset(m_bases, 0, sizeof(m_bases));
	m_useconfig_14x = false;
	m_useconfig_18x = false;
}

void sega_315_6154_device::device_start()
{
	pci_host_device::device_start();
	set_spaces(&space(AS_PCI_MEMORY));
	// never unmap addresses lower than start
	memory_window_start = 0xc0000000;
	memory_window_end =   0xffffffff;
	memory_offset = 0;

	m_configuration = &space(AS_PCI_CONFIG);

	io_window_start = 0xc0000000;
	io_window_end = 0xc000ffff;
	io_offset = 0xc0000000;

	save_item(NAME(m_registers));
	save_item(NAME(m_bases));
	save_item(NAME(m_useconfig_14x));
	save_item(NAME(m_useconfig_18x));
	save_item(NAME(m_mode));
}

void sega_315_6154_device::device_reset()
{
	pci_host_device::device_reset();
	memset(m_registers, 0, sizeof(m_registers));
}

device_memory_interface::space_config_vector sega_315_6154_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PCI_CONFIG, &m_configuration_config),
		std::make_pair(AS_PCI_MEMORY, &m_memory_config)
	};
}

void sega_315_6154_device::regenerate_config_mapping()
{
	// like pci_bridge_device::regenerate_config_mapping() in pci.cpp
	// but each device n is assigned bit n+11 in the configuration space
	// devices in different buses must have different numbers
	address_space *config_space = &space(AS_PCI_CONFIG);
	config_space->unmap_readwrite(0x00000, 0xfffff);
	for (int i = 0; i < 9; i++)
		if (sub_devices[i])
		{
			const int s = i >> 3;
			const int o = (i & 7) << 8;
			config_space->install_device((0x800 << s) + o, ((0x800 << s) + o) | 0xff, *sub_devices[i], &pci_device::config_map);
		}
}

void sega_315_6154_device::irq_update()
{
	irq1_cb(((m_registers[1] & 0xffff) & (m_registers[1] >> 16)) ? ASSERT_LINE : CLEAR_LINE);
}

u32 sega_315_6154_device::registers_r(offs_t offset)
{
	if (offset == 0)
		return m_mode;

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

	// hacky mask out top address bit
	for (int i = 0; i < 4; i++)
		if (base[i] < 0xc0000000)
			base[i] &= 0x7fffffff;
}

void sega_315_6154_device::registers_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x04 / 4)
	{
		m_registers[1] &= ~(data & mem_mask & 0xffff);
		mem_mask &= 0xffff0000;
		COMBINE_DATA(m_registers + 1);
		irq_update();
		return;
	}

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
		if (ACCESSING_BITS_24_31 && BIT(m_registers[0x38 / 4], 24))
		{
			uint32_t s, d, l;

			s = m_registers[0x30 / 4];
			d = m_registers[0x34 / 4];
			l = m_registers[0x38 / 4] & 0xffffff;
			logerror("got dma transfer request from 0x%08x to 0x%08x size 0x%08x bytes\n", s, d, l << 2);

			// for some reason bit 31 always(?) set when accessing local bus area via DMA, lets clear it
			if (s < 0xc0000000)
				s &= 0x7fffffff;
			if (d < 0xc0000000)
				d &= 0x7fffffff;

			while (l != 0)
			{
				space(AS_PCI_MEMORY).write_dword(d, space(AS_PCI_MEMORY).read_dword(s));
				s += 4;
				d += 4;
				l--;
			}

			// TODO: this should be delayed
			m_registers[0x38 / 4] &= ~(1 << 24);
			m_registers[1] |= 1 << 12;
			irq_update();
		}
	}
}

template<int Aperture>
u32 sega_315_6154_device::aperture_r(offs_t offset, u32 mem_mask)
{
	const u32 destination_offset = offset & 0x3fffff;
	const int destination = (offset >> 22) & 3;
	const int index = Aperture * 4 + destination;

	if ((Aperture == 0) && (destination == 0) && (m_useconfig_14x == true))
		return m_configuration->read_dword(destination_offset << 2, mem_mask);
	if ((Aperture == 1) && (destination == 0) && (m_useconfig_18x == true))
		return m_configuration->read_dword(destination_offset << 2, mem_mask);
	return space(AS_PCI_MEMORY).read_dword(m_bases[index] + (destination_offset << 2), mem_mask);
}

template u32 sega_315_6154_device::aperture_r<0>(offs_t offset, u32 mem_mask);
template u32 sega_315_6154_device::aperture_r<1>(offs_t offset, u32 mem_mask);
template u32 sega_315_6154_device::aperture_r<2>(offs_t offset, u32 mem_mask);

template<int Aperture>
void sega_315_6154_device::aperture_w(offs_t offset, u32 data, u32 mem_mask)
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
	space(AS_PCI_MEMORY).write_dword(m_bases[index] + (destination_offset << 2), data, mem_mask);
}

template void sega_315_6154_device::aperture_w<0>(offs_t offset, u32 data, u32 mem_mask);
template void sega_315_6154_device::aperture_w<1>(offs_t offset, u32 data, u32 mem_mask);
template void sega_315_6154_device::aperture_w<2>(offs_t offset, u32 data, u32 mem_mask);
