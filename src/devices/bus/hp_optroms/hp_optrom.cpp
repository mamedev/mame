// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp_optrom.cpp

    Optional ROMs for HP9845 systems

*********************************************************************/

#include "hp_optrom.h"
#include "softlist.h"
#include "cpu/hphybrid/hphybrid.h"

const device_type HP_OPTROM_CART = &device_creator<hp_optrom_cart_device>;
const device_type HP_OPTROM_SLOT = &device_creator<hp_optrom_slot_device>;

// +---------------------+
// |hp_optrom_cart_device|
// +---------------------+
hp_optrom_cart_device::hp_optrom_cart_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_card_interface(mconfig, *this)
{
}

hp_optrom_cart_device::hp_optrom_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, HP_OPTROM_CART, "HP9845 optional ROM cartridge", tag, owner, clock, "hp_optrom_cart", __FILE__),
		device_slot_card_interface(mconfig, *this)
{
}

// +---------------------+
// |hp_optrom_slot_device|
// +---------------------+
hp_optrom_slot_device::hp_optrom_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, HP_OPTROM_SLOT, "HP9845 optional ROM Slot", tag, owner, clock, "hp_optrom_slot", __FILE__),
		device_image_interface(mconfig, *this),
		device_slot_interface(mconfig, *this),
		m_cart(nullptr),
		m_base_addr(0),
		m_end_addr(0)
{
}

hp_optrom_slot_device::~hp_optrom_slot_device()
{
}

void hp_optrom_slot_device::device_start()
{
		m_cart = dynamic_cast<hp_optrom_cart_device *>(get_card_device());
}

void hp_optrom_slot_device::device_config_complete()
{
		update_names(HP_OPTROM_SLOT , "optional_rom" , "optrom");
}

image_init_result hp_optrom_slot_device::call_load()
{
		logerror("hp_optrom: call_load\n");
		if (m_cart == nullptr || !loaded_through_softlist()) {
				logerror("hp_optrom: must be loaded from sw list\n");
				return image_init_result::FAIL;
		}

		const char *base_feature = get_feature("base");
		if (base_feature == nullptr) {
				logerror("hp_optrom: no 'base' feature\n");
				return image_init_result::FAIL;
		}

		offs_t base_addr;
		if (base_feature[ 0 ] != '0' || base_feature[ 1 ] != 'x' || sscanf(&base_feature[ 2 ] , "%x" , &base_addr) != 1) {
				logerror("hp_optrom: can't parse 'base' feature\n");
				return image_init_result::FAIL;
		}

		// Valid BSC values for ROMs on LPU drawer: 0x07 0x0b .... 0x3b
		// Valid BSC values for ROMs on PPU drawer: 0x09 0x0d .... 0x3d
		// (BSC is field in bits 16..21 of base address)
		// Bit 15 of base address must be 0
		// Base address must be multiple of 0x1000
		if ((base_addr & ~0x3f7000UL) != 0 || ((base_addr & 0x30000) != 0x10000 && (base_addr & 0x30000) != 0x30000) || base_addr < 0x70000) {
				logerror("hp_optrom: illegal base address (%x)\n" , base_addr);
				return image_init_result::FAIL;
		}

		auto length = get_software_region_length("rom") / 2;

		if (length < 0x1000 || length > 0x8000 || (length & 0xfff) != 0 || ((base_addr & 0x7000) + length) > 0x8000) {
				logerror("hp_optrom: illegal region length (%x)\n" , length);
				return image_init_result::FAIL;
		}

		offs_t end_addr = base_addr + length - 1;
		logerror("hp_optrom: base_addr = %06x end_addr = %06x\n" , base_addr , end_addr);

		m_content.resize(length * 2);
		UINT8 *buffer = m_content.data();
		memcpy(buffer , get_software_region("rom") , length * 2);

		// Install ROM in address space of every CPU
		for (hp_hybrid_cpu_device& cpu : device_interface_iterator<hp_hybrid_cpu_device>(machine().root_device())) {
				logerror("hp_optrom: install in %s AS\n" , cpu.tag());
				cpu.space(AS_PROGRAM).install_rom(base_addr , end_addr , buffer);
		}

		m_base_addr = base_addr;
		m_end_addr = end_addr;

		return image_init_result::PASS;
}

void hp_optrom_slot_device::call_unload()
{
		logerror("hp_optrom: call_unload\n");
		if (m_cart != nullptr && m_base_addr != 0 && m_end_addr != 0) {
				for (hp_hybrid_cpu_device& cpu : device_interface_iterator<hp_hybrid_cpu_device>(machine().root_device())) {
						cpu.space(AS_PROGRAM).unmap_read(m_base_addr , m_end_addr);
				}
				m_content.resize(0);
				m_base_addr = 0;
				m_end_addr = 0;
		}
}

std::string hp_optrom_slot_device::get_default_card_software()
{
		return software_get_default_slot("rom");
}

SLOT_INTERFACE_START(hp_optrom_slot_device)
		SLOT_INTERFACE_INTERNAL("rom", HP_OPTROM_CART)
SLOT_INTERFACE_END
