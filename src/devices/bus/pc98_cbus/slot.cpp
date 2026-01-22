// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

C-bus slot interface for PC-98xx family

a.k.a. NEC version of the ISA bus.
C-bus -> Compatible Bus

References:
- https://98epjunk.shakunage.net/sw/ext_card/ext_card.html
- https://ja.wikipedia.org/wiki/C%E3%83%90%E3%82%B9
- https://www.pc-9800.net/db2/db2_ga_index.htm
- https://web.archive.org/web/20240921232349/https://radioc.web.fc2.com/column/pc98bas/pc98memmap_en.htm
- http://ookumaneko.s1005.xrea.com/pcibios.htm (PCI era mapping)

TODO:
- 8-bit I/O smearing will eventually be a problem i.e. cfr. current fdd_2d position in host.
  Consider narrowing range or make address_maps having a "transparent pen" on umaskN zeroes.
- DMA;
- Move 2HD / 2DD 1st gen FDC;
- Move 2HD/2DD combined FDC for later HW;
- Move IDE BIOSes;
- Subscribe to I/O $43f handling, add a category subscription for specific cards (SASI and SCSI);
- Support for PCI bridging on later machines (cfr. pc9821cx3)
\- "local bus bridge", handles RAM stuff on its own, definitely not a pure southbridge.
\- set_ids_bridge(0x10330001, <rev>, 0x068000, 0x10330001) should be for 486 targets
\- set_ids_bridge(0x10330002, <rev>, 0x068000, 0x10330002) for Pentium

**************************************************************************************************/

#include "emu.h"

#include "slot.h"


DEFINE_DEVICE_TYPE(PC98_CBUS_ROOT, pc98_cbus_root_device, "pc98_cbus_root", "PC-98 C-Bus root")
DEFINE_DEVICE_TYPE(PC98_CBUS_SLOT, pc98_cbus_slot_device, "pc98_cbus_slot", "PC-98 C-Bus slot")

//**************************************************************************
//  DEVICE PC9801 ROOT INTERFACE
//**************************************************************************

pc98_cbus_root_device::pc98_cbus_root_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PC98_CBUS_ROOT, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_mem_config("mem_space", ENDIANNESS_LITTLE, 16, 24, 0, address_map_constructor())
	, m_space_io_config("io_space", ENDIANNESS_LITTLE, 16, 16, 0, address_map_constructor())
	, m_int_cb(*this)
	, m_drq_cb(*this)
{
	std::fill(std::begin(m_dma_device), std::end(m_dma_device), nullptr);
	std::fill(std::begin(m_dma_eop), std::end(m_dma_eop), false);
}

device_memory_interface::space_config_vector pc98_cbus_root_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_mem_config),
		std::make_pair(AS_IO, &m_space_io_config)
	};
}


void pc98_cbus_root_device::device_start()
{
	space(AS_PROGRAM).unmap_value_high();
	space(AS_IO).unmap_value_high();
}

void pc98_cbus_root_device::device_reset()
{
	logerror("Unmap memory ranges\n");
	space(AS_PROGRAM).unmap_readwrite(0, 0xffffff);
	space(AS_IO).unmap_readwrite(0, 0xffff);
}

void pc98_cbus_root_device::device_reset_after_children()
{
	logerror("Remap card ranges\n");
	remap(AS_PROGRAM, 0, 0xffffff);
	remap(AS_IO, 0, 0xffff);
}

void pc98_cbus_root_device::device_config_complete()
{
	// ...
}

void pc98_cbus_root_device::add_slot(const char *tag)
{
	device_t *dev = subdevice(tag);
	add_slot(dynamic_cast<device_slot_interface *>(dev));
}

void pc98_cbus_root_device::add_slot(device_slot_interface *slot)
{
	m_slot_list.push_front(slot);
}

void pc98_cbus_root_device::remap(int space_id, offs_t start, offs_t end)
{
	for (device_slot_interface *sl : m_slot_list)
	{
		device_t *dev = sl->get_card_device();
		if (dev)
		{
			device_pc98_cbus_slot_interface *cbusdev = dynamic_cast<device_pc98_cbus_slot_interface *>(dev);
			cbusdev->remap(space_id, start, end);
		}
	}
}

// for RAM in conventional memory space in x86 land.
u16 pc98_cbus_root_device::mem_r(offs_t offset, u16 mem_mask)
{
	return space(AS_PROGRAM).read_word(offset << 1, mem_mask);
}

void pc98_cbus_root_device::mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	space(AS_PROGRAM).write_word(offset << 1, data, mem_mask);
}

// alias for ROM/RAM in $c0000-$dffff space
u16 pc98_cbus_root_device::mem_slot_r(offs_t offset, u16 mem_mask)
{
	return space(AS_PROGRAM).read_word((offset << 1) + 0xc'0000, mem_mask);
}

void pc98_cbus_root_device::mem_slot_w(offs_t offset, u16 data, u16 mem_mask)
{
	space(AS_PROGRAM).write_word((offset << 1) + 0xc'0000, data, mem_mask);
}

u16 pc98_cbus_root_device::io_r(offs_t offset, u16 mem_mask)
{
	return space(AS_IO).read_word(offset << 1, mem_mask);
}

void pc98_cbus_root_device::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	space(AS_IO).write_word(offset << 1, data, mem_mask);
}

u8 pc98_cbus_root_device::dack_r(int line)
{
	if (m_dma_device[line])
		return m_dma_device[line]->dack_r(line);
	return 0xff;
}

void pc98_cbus_root_device::dack_w(int line, u8 data)
{
	if (m_dma_device[line])
		m_dma_device[line]->dack_w(line, data);
}

void pc98_cbus_root_device::eop_w(int line, int state)
{
	if (m_dma_eop[line] && m_dma_device[line])
		m_dma_device[line]->eop_w(state);
}

void pc98_cbus_root_device::set_dma_channel(u8 channel, device_pc98_cbus_slot_interface *dev, bool do_eop)
{
	m_dma_device[channel] = dev;
	m_dma_eop[channel] = do_eop;
}


//**************************************************************************
//  DEVICE PC9801 CARD INTERFACE
//**************************************************************************

device_pc98_cbus_slot_interface::device_pc98_cbus_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pc98_cbus")
{
}

device_pc98_cbus_slot_interface::~device_pc98_cbus_slot_interface()
{
}

u8 device_pc98_cbus_slot_interface::dack_r(int line)
{
	return 0;
}

void device_pc98_cbus_slot_interface::dack_w(int line, u8 data)
{
}

void device_pc98_cbus_slot_interface::eop_w(int state)
{
}


pc98_cbus_slot_device::pc98_cbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pc98_cbus_slot_device(mconfig, PC98_CBUS_SLOT, tag, owner, clock)
{
}

pc98_cbus_slot_device::pc98_cbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}

void pc98_cbus_slot_device::device_start()
{
	device_pc98_cbus_slot_interface *const dev = dynamic_cast<device_pc98_cbus_slot_interface *>(get_card_device());
	if (dev) dev->set_bus(m_bus);
	dynamic_cast<pc98_cbus_root_device &>(*m_bus).add_slot(tag());
}
