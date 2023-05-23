// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*
    OPTi 82C861 "FireLink" USB 1.1 OHCI controller
    Skeleton by R. Belmont
*/

#include "emu.h"
#include "opti82c861.h"

#define LOG_REGISTERS   (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(OPTI_82C861, opti_82c861_device, "opti82c861", "OPTi 82C861 USB OHCI controller")

opti_82c861_device::opti_82c861_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, type, tag, owner, clock)
{
}

opti_82c861_device::opti_82c861_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: opti_82c861_device(mconfig, OPTI_82C861, tag, owner, clock)
{
}

void opti_82c861_device::mem_map(address_map& map)
{
}

void opti_82c861_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

void opti_82c861_device::device_start()
{
	pci_device::device_start();
	set_ids(0x1045c861, 0x10, 0x0c0310, 0);
	revision = 0x10;
	add_map(0x1000, M_MEM, FUNC(opti_82c861_device::mem_map));  // 4KiB memory map

	command = 0;
	intr_pin = 1;
	intr_line = 0;
}

void opti_82c861_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
}
