// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*
    OPTi 82C861 "FireLink" USB 1.1 OHCI controller
    Skeleton by R. Belmont
*/

#include "emu.h"
#include "opti82c861.h"

#define LOG_REGISTERS   (1U << 1)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(OPTI_82C861, opti_82c861_device, "opti82c861", "OPTi 82C861 \"FireLink\" USB OHCI controller")

opti_82c861_device::opti_82c861_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
}

opti_82c861_device::opti_82c861_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: opti_82c861_device(mconfig, OPTI_82C861, tag, owner, clock)
{
}

void opti_82c861_device::mem_map(address_map& map)
{
	// HcRevision: OpenHCI v1.0 with legacy support
	map(0x000, 0x003).lr32(NAME([]() { return 0x00000110; }));
	map(0x048, 0x04b).lrw32(
		NAME([this] () {
			// 2 downstream ports (always?)
			return (m_HcRhDescriptorA & 0xff001b00) | 0x02;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_HcRhDescriptorA);
			if (ACCESSING_BITS_24_31)
				LOG("HcRhDescriptorA: set Power-On to Power-Good Time %d msec\n", (m_HcRhDescriptorA >> 24) * 2); 
			if (ACCESSING_BITS_8_15)
				LOG("HcRhDescriptorA: set status %02x\n", (m_HcRhDescriptorA & 0x1b00) >> 8);
		})
	);
}

void opti_82c861_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//	map(0x4e, 0x4e) i2c Control Register
//	map(0x50, 0x50) PCI Host Feature Control Register
//	map(0x51, 0x51) Interrupt Assignment Register
//	map(0x52, 0x52) Strap Option Enable
//	map(0x54, 0x57) IRQ Driveback Address Register
//	map(0x6c, 0x6f) Test Mode Enable Register
}

void opti_82c861_device::device_start()
{
	pci_card_device::device_start();
	set_ids(0x1045c861, 0x10, 0x0c0310, 0);
	revision = 0x10;
	add_map(0x1000, M_MEM, FUNC(opti_82c861_device::mem_map));  // 4KiB memory map

	command = 0;
	// fast back-to-back, medium DEVSEL#
	status = 0x0280;
	intr_pin = 1;
	intr_line = 0;
}

void opti_82c861_device::device_reset()
{
	pci_card_device::device_reset();
	m_HcRhDescriptorA = 0x01000000;
}

void opti_82c861_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}
