// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Konami 033906 SHARC-to-PCI host bridge

    The configuration aperture accesses the single graphics device at 00.0.
    The SHARC's 02400000-027fffff word-addressed window accesses PCI memory
    at 00000000-00ffffff.  Hornet firmware tests BAR0, then programs it to
    zero and enables memory decoding before accessing this window.

***************************************************************************/

#include "emu.h"
#include "k033906.h"

DEFINE_DEVICE_TYPE(K033906, k033906_device, "k033906", "Konami 033906 PCI host bridge")

k033906_device::k033906_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, K033906, tag, owner, clock)
	, m_memory_config("memory_space", ENDIANNESS_LITTLE, 32, 32)
	, m_io_config("io_space", ENDIANNESS_LITTLE, 32, 32)
	, m_ram(*this, "ram", 0x20000, ENDIANNESS_LITTLE)
	, m_reg_set(0)
{
}

void k033906_device::device_start()
{
	pci_host_device::device_start();
	set_spaces(&space(AS_PCI_MEM), &space(AS_PCI_IO));
	space(AS_PCI_MEM).unmap_value_high();
	space(AS_PCI_MEM).specific(m_memory);
	space(AS_PCI_IO).unmap_value_high();

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffffffff;
	io_offset = 0;

	save_item(NAME(m_reg_set));
}

device_memory_interface::space_config_vector k033906_device::memory_space_config() const
{
	auto spaces = pci_host_device::memory_space_config();
	spaces.emplace_back(AS_PCI_MEM, &m_memory_config);
	spaces.emplace_back(AS_PCI_IO, &m_io_config);
	return spaces;
}

void k033906_device::set_reg(int state)
{
	m_reg_set = state & 1;
}

u32 k033906_device::read(offs_t offset, u32 mem_mask)
{
	if (m_reg_set)
		return (offset < 0x40) ? root_config_read(0, 0, offset * 4, mem_mask) : 0xffffffff;
	else
		return m_ram[offset];
}

void k033906_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_reg_set)
	{
		if (offset < 0x40)
			root_config_write(0, 0, offset * 4, data, mem_mask);
	}
	else
	{
		COMBINE_DATA(&m_ram[offset]);
	}
}

u32 k033906_device::memory_r(offs_t offset, u32 mem_mask)
{
	return m_memory.read_dword(offset * 4, mem_mask);
}

void k033906_device::memory_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_memory.write_dword(offset * 4, data, mem_mask);
}
