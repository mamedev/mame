// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp80_optrom.cpp

    Optional ROMs for HP80 systems

*********************************************************************/

#include "emu.h"
#include "hp80_optrom.h"
#include "softlist.h"

// Debugging
#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(HP80_OPTROM_CART, hp80_optrom_cart_device, "hp80_optrom_cart", "HP80 optional ROM cartridge")
DEFINE_DEVICE_TYPE(HP80_OPTROM_SLOT, hp80_optrom_slot_device, "hp80_optrom_slot", "HP80 optional ROM slot")

// +-----------------------+
// |hp80_optrom_cart_device|
// +-----------------------+
hp80_optrom_cart_device::hp80_optrom_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_slot_card_interface(mconfig, *this)
{
}

hp80_optrom_cart_device::hp80_optrom_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	hp80_optrom_cart_device(mconfig, HP80_OPTROM_CART, tag, owner, clock)
{
}

// +-----------------------+
// |hp80_optrom_slot_device|
// +-----------------------+
hp80_optrom_slot_device::hp80_optrom_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP80_OPTROM_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_slot_interface(mconfig, *this),
	m_cart(nullptr),
	m_select_code(0)
{
}

hp80_optrom_slot_device::~hp80_optrom_slot_device()
{
}

void hp80_optrom_slot_device::install_read_handler(address_space& space)
{
	if (loaded_through_softlist()) {
		offs_t start = (offs_t)m_select_code * HP80_OPTROM_SIZE;
		space.install_rom(start , start + HP80_OPTROM_SIZE - 1 , get_software_region("rom"));
	}
}

void hp80_optrom_slot_device::device_start()
{
	m_cart = dynamic_cast<hp80_optrom_cart_device *>(get_card_device());
}

image_init_result hp80_optrom_slot_device::call_load()
{
	LOG("hp80_optrom: call_load\n");
	if (m_cart == nullptr || !loaded_through_softlist()) {
		LOG("hp80_optrom: must be loaded from sw list\n");
		return image_init_result::FAIL;
	}

	const char *sc_feature = get_feature("sc");
	if (sc_feature == nullptr) {
		LOG("hp80_optrom: no 'sc' feature\n");
		return image_init_result::FAIL;
	}

	unsigned sc;
	if (sc_feature[ 0 ] != '0' || sc_feature[ 1 ] != 'x' || sscanf(&sc_feature[ 2 ] , "%x" , &sc) != 1) {
		LOG("hp80_optrom: can't parse 'sc' feature\n");
		return image_init_result::FAIL;
	}

	// Valid SC values: 0x01..0xff
	if (sc < 1 || sc > 0xff) {
		LOG("hp80_optrom: illegal select code (%x)\n" , sc);
		return image_init_result::FAIL;
	}

	auto length = get_software_region_length("rom");

	if (length != HP80_OPTROM_SIZE) {
		LOG("hp80_optrom: illegal region length (%x)\n" , length);
		return image_init_result::FAIL;
	}

	LOG("hp80_optrom: loaded SC=0x%02x\n" , sc);
	m_select_code = sc;
	return image_init_result::PASS;
}

void hp80_optrom_slot_device::call_unload()
{
	LOG("hp80_optrom: call_unload\n");
	machine().schedule_soft_reset();
}

std::string hp80_optrom_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}

void hp80_optrom_slot_devices(device_slot_interface &device)
{
	device.option_add_internal("rom", HP80_OPTROM_CART);
}
