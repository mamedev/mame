// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp_ipc_optrom.cpp

    Optional ROMs for HP Integral PC

*********************************************************************/

#include "emu.h"
#include "hp_ipc_optrom.h"
#include "softlist.h"

// Debugging
#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(HP_IPC_OPTROM, hp_ipc_optrom_device, "hp_ipc_optrom", "HP IPC optional ROM")

// +--------------------+
// |hp_ipc_optrom_device|
// +--------------------+
hp_ipc_optrom_device::hp_ipc_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP_IPC_OPTROM, tag, owner, clock)
	, device_rom_image_interface(mconfig, *this)
	, m_base(0)
{
}

hp_ipc_optrom_device::~hp_ipc_optrom_device()
{
}

void hp_ipc_optrom_device::install_read_handler(address_space& space)
{
	if (loaded_through_softlist()) {
		auto region = get_software_region("rom");
		auto len = get_software_region_length("rom");

		// Supervisor mode
		space.install_rom(m_base , m_base + len - 1 , region);
		// User mode
		space.install_rom(m_base + 0x1800000 , m_base + 0x1800000 + len - 1 , region);
	}
}

void hp_ipc_optrom_device::device_start()
{
}

image_init_result hp_ipc_optrom_device::call_load()
{
	LOG("hp_ipc_optrom: call_load\n");
	if (!loaded_through_softlist()) {
		LOG("hp_ipc_optrom: must be loaded from sw list\n");
		return image_init_result::FAIL;
	}

	const char *base_feature = get_feature("base");
	if (base_feature == nullptr) {
		LOG("hp_ipc_optrom: no 'base' feature\n");
		return image_init_result::FAIL;
	}

	if (base_feature[ 0 ] != '0' || base_feature[ 1 ] != 'x' || sscanf(&base_feature[ 2 ] , "%x" , &m_base) != 1) {
		LOG("hp_ipc_optrom: can't parse 'base' feature\n");
		return image_init_result::FAIL;
	}

	// Valid base values: multiple of 1M, [0x100000..0x4fffff]
	if ((m_base & 0xfffffU) != 0 || m_base < 0x100000U || m_base > 0x400000U) {
		LOG("hp_ipc_optrom: illegal base (%x)\n" , m_base);
		return image_init_result::FAIL;
	}

	LOG("hp_ipc_optrom: loaded base=0x%06x\n" , m_base);
	return image_init_result::PASS;
}

void hp_ipc_optrom_device::call_unload()
{
	LOG("hp_ipc_optrom: call_unload\n");
}
