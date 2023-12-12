// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

[Cyrix/National Semiconductor/AMD] [MediaGX/Geode] [Cx/CS]5530 bridge implementation (southbridge)

**************************************************************************************************/

#include "emu.h"
#include "mediagx_cs5530_bridge.h"

DEFINE_DEVICE_TYPE(MEDIAGX_CS5530_BRIDGE, mediagx_cs5530_bridge_device, "mediagx_cs5530_bridge", "MediaGX CS5530 Bridge")

mediagx_cs5530_bridge_device::mediagx_cs5530_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MEDIAGX_CS5530_BRIDGE, tag, owner, clock)
//	, m_smi_callback(*this)
//	, m_nmi_callback(*this)
//	, m_stpclk_callback(*this)
	, m_boot_state_hook(*this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
//	, m_pic8259_master(*this, "pic8259_master")
//	, m_pic8259_slave(*this, "pic8259_slave")
//	, m_dma8237_1(*this, "dma8237_1")
//	, m_dma8237_2(*this, "dma8237_2")
//	, m_pit8254(*this, "pit8254")
	, m_isabus(*this, "isabus")
//	, m_speaker(*this, "speaker")
{
}

void mediagx_cs5530_bridge_device::device_add_mconfig(machine_config &config)
{
	ISA16(config, m_isabus, 0);
}

void mediagx_cs5530_bridge_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_maincpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_maincpu, AS_IO);

	pci_device::device_config_complete();
}

void mediagx_cs5530_bridge_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0280;
}

void mediagx_cs5530_bridge_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// ...
}

void mediagx_cs5530_bridge_device::internal_io_map(address_map &map)
{
//	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
//	map(0x0020, 0x003f).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
//	map(0x0040, 0x005f).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
//	map(0x0061, 0x0061).rw(FUNC(mediagx_cs5530_bridge_device::at_portb_r), FUNC(mediagx_cs5530_bridge_device::at_portb_w));
//	map(0x0070, 0x0071) RTC
//	map(0x0080, 0x009f).rw(FUNC(mediagx_cs5530_bridge_device::at_page8_r), FUNC(mediagx_cs5530_bridge_device::at_page8_w));
//	map(0x00a0, 0x00bf).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
//	map(0x00c0, 0x00df).rw(FUNC(mediagx_cs5530_bridge_device::at_dma8237_2_r), FUNC(mediagx_cs5530_bridge_device::at_dma8237_2_w));
//	map(0x04d0, 0x04d1).rw(FUNC(mediagx_cs5530_bridge_device::eisa_irq_read), FUNC(mediagx_cs5530_bridge_device::eisa_irq_write));
	map(0x00e0, 0x00ef).noprw();
//	map(0x121c, 0x121f) ACPI Timer count register (on rev 1.3+)
}

void mediagx_cs5530_bridge_device::map_bios(address_space *memory_space, uint32_t start, uint32_t end)
{
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(start, end, m_region->base() + (start & mask));
}

void mediagx_cs5530_bridge_device::map_extra(
		uint64_t memory_window_start,
		uint64_t memory_window_end,
		uint64_t memory_offset,
		address_space *memory_space,
		uint64_t io_window_start,
		uint64_t io_window_end,
		uint64_t io_offset,
		address_space *io_space)
{
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	map_bios(memory_space, 0xffffffff - m_region->bytes() + 1, 0xffffffff);
	// TODO: BIOS window conditions
	map_bios(memory_space, 0x000e0000, 0x000fffff);
	m_isabus->remap(AS_IO, 0, 0xffff);
	io_space->install_device(0, 0xffff, *this, &mediagx_cs5530_bridge_device::internal_io_map);
}


