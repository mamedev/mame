// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_optrom.cpp

    Optional ROMs for HP9845 systems

*********************************************************************/

#include "emu.h"
#include "hp9845_optrom.h"
#include "softlist.h"
#include "cpu/hphybrid/hphybrid.h"

DEFINE_DEVICE_TYPE(HP9845_OPTROM, hp9845_optrom_device, "hp9845_optrom", "HP9845 optional ROM")

// +--------------------+
// |hp9845_optrom_device|
// +--------------------+
hp9845_optrom_device::hp9845_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, HP9845_OPTROM, tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_base_addr(0),
		m_end_addr(0)
{
}

hp9845_optrom_device::~hp9845_optrom_device()
{
}

void hp9845_optrom_device::device_start()
{
}

image_init_result hp9845_optrom_device::call_load()
{
	logerror("hp9845_optrom: call_load\n");
	if (!loaded_through_softlist()) {
		logerror("hp9845_optrom: must be loaded from sw list\n");
		return image_init_result::FAIL;
	}

	const char *base_feature = get_feature("base");
	if (base_feature == nullptr) {
		logerror("hp9845_optrom: no 'base' feature\n");
		return image_init_result::FAIL;
	}

	offs_t base_addr;
	if (base_feature[ 0 ] != '0' || base_feature[ 1 ] != 'x' || sscanf(&base_feature[ 2 ] , "%x" , &base_addr) != 1) {
		logerror("hp9845_optrom: can't parse 'base' feature\n");
		return image_init_result::FAIL;
	}

	// Valid BSC values for ROMs on LPU drawer: 0x07 0x0b .... 0x3b
	// Valid BSC values for ROMs on PPU drawer: 0x09 0x0d .... 0x3d
	// (BSC is field in bits 16..21 of base address)
	// Bit 15 of base address must be 0
	// Base address must be multiple of 0x1000
	if ((base_addr & ~0x3f7000UL) != 0 || ((base_addr & 0x30000) != 0x10000 && (base_addr & 0x30000) != 0x30000) || base_addr < 0x70000) {
		logerror("hp9845_optrom: illegal base address (%x)\n" , base_addr);
		return image_init_result::FAIL;
	}

	auto length = get_software_region_length("rom") / 2;

	if (length < 0x1000 || length > 0x8000 || (length & 0xfff) != 0 || ((base_addr & 0x7000) + length) > 0x8000) {
		logerror("hp9845_optrom: illegal region length (%x)\n" , length);
		return image_init_result::FAIL;
	}

	offs_t end_addr = base_addr + length - 1;
	logerror("hp9845_optrom: base_addr = %06x end_addr = %06x\n" , base_addr , end_addr);

	// Install ROM in address space of every CPU
	for (hp_hybrid_cpu_device& cpu : device_interface_iterator<hp_hybrid_cpu_device>(machine().root_device())) {
		logerror("hp9845_optrom: install in %s AS\n" , cpu.tag());
		cpu.space(AS_PROGRAM).install_rom(base_addr , end_addr , get_software_region("rom"));
	}

	m_base_addr = base_addr;
	m_end_addr = end_addr;

	return image_init_result::PASS;
}

void hp9845_optrom_device::call_unload()
{
	logerror("hp9845_optrom: call_unload\n");
	if (m_base_addr != 0 && m_end_addr != 0) {
		for (hp_hybrid_cpu_device& cpu : device_interface_iterator<hp_hybrid_cpu_device>(machine().root_device())) {
			cpu.space(AS_PROGRAM).unmap_read(m_base_addr , m_end_addr);
		}
		m_base_addr = 0;
		m_end_addr = 0;
	}
}
