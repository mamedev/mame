// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9825_optrom.cpp

    Optional ROMs for HP9825 systems

*********************************************************************/

#include "emu.h"
#include "hp9825_optrom.h"
#include "softlist.h"

// Debugging
//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(HP9825_OPTROM_CART, hp9825_optrom_cart_device, "hp9825_optrom_cart", "HP9825 optional ROM cartridge")
DEFINE_DEVICE_TYPE(HP9825_OPTROM_SLOT, hp9825_optrom_slot_device, "hp9825_optrom_slot", "HP9825 optional ROM slot")

struct optrom_region {
	offs_t m_start;
	offs_t m_size;
	const char *m_tag;
};

constexpr std::array<struct optrom_region , 8> region_tab =
	{{
	  { 0x3000 , 0x400 , "rom3000" },
	  { 0x3400 , 0x400 , "rom3400" },
	  { 0x3800 , 0x400 , "rom3800" },
	  { 0x3c00 , 0x400 , "rom3c00" },
	  { 0x4000 , 0x400 , "rom4000" },
	  { 0x4400 , 0x800 , "rom4400" },
	  { 0x4c00 , 0x400 , "rom4c00" },
	  { 0x5c00 ,0x2000 , "rom5c00" }
	}};

// +-------------------------+
// |hp9825_optrom_cart_device|
// +-------------------------+
hp9825_optrom_cart_device::hp9825_optrom_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp9825_optrom_cart_device(mconfig , HP9825_OPTROM_CART , tag , owner , clock)
{
}

hp9825_optrom_cart_device::hp9825_optrom_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_card_interface(mconfig, *this)
{
}

// +-------------------------+
// |hp9825_optrom_slot_device|
// +-------------------------+
hp9825_optrom_slot_device::hp9825_optrom_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
	: hp9825_optrom_slot_device(mconfig, tag, owner, (uint32_t)0)
{
}

hp9825_optrom_slot_device::hp9825_optrom_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP9825_OPTROM_SLOT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, device_slot_interface(mconfig, *this)
	, m_cart(nullptr)
	, m_rom_limit(0xffffU)
	, m_loaded_regions(0)
	, m_space_r(nullptr)
	, m_bank(*this , "rombank")
{
	option_reset();
	option_add_internal("rom", HP9825_OPTROM_CART);
	set_default_option(nullptr);
	set_fixed(false);
}

hp9825_optrom_slot_device::~hp9825_optrom_slot_device()
{
}

void hp9825_optrom_slot_device::install_rw_handlers(address_space *space_r , address_space *space_w)
{
	LOG("limit=%04x\n" , m_rom_limit);
	m_loaded_regions = 0;
	m_space_r = space_r;

	unsigned mask = 1;

	for (const struct optrom_region& reg : region_tab) {
		uint8_t *ptr = get_software_region(reg.m_tag);
		if (ptr != nullptr) {
			LOG("%s loaded\n" , reg.m_tag);
			if (reg.m_start == 0x5c00) {
				space_r->install_rom(0x3000 , 0x33ff , ptr);
				space_r->install_device(0x5c00 , 0x5fff , *m_bank , &address_map_bank_device::amap16);
				m_bank->space(AS_PROGRAM).install_rom(0 , 0x1fff , ptr);
			} else {
				space_r->install_rom(reg.m_start , reg.m_start + reg.m_size - 1 , ptr);
			}
			m_loaded_regions |= mask;
		}
		mask <<= 1;
	}
	if (space_w != nullptr) {
		space_w->install_write_tap(0 , 0x3ff , "bank_switch" ,
								   [this](offs_t offset, u16 &data, u16 mem_mask) {
									   if (BIT(offset , 5)) {
										   LOG("bank = %u\n" , offset & 7);
										   m_bank->set_bank(offset & 7);
									   }
								   });
	}
}

void hp9825_optrom_slot_device::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config , m_bank).set_options(ENDIANNESS_BIG , 16 , 13 , 0x400).set_shift(-1);
}

void hp9825_optrom_slot_device::device_start()
{
	m_cart = dynamic_cast<hp9825_optrom_cart_device *>(get_card_device());
}

image_init_result hp9825_optrom_slot_device::call_load()
{
	LOG("hp9825_optrom: call_load\n");
	if (m_cart == nullptr || !loaded_through_softlist()) {
		LOG("hp9825_optrom: must be loaded from sw list\n");
		return image_init_result::FAIL;
	}

	for (const struct optrom_region& reg : region_tab) {
		auto len = get_software_region_length(reg.m_tag) / 2;
		if (len != 0) {
			if (len != reg.m_size) {
				LOG("Region %s: wrong size (%u should be %u)\n" , reg.m_tag , len , reg.m_size);
				return image_init_result::FAIL;
			}
			if (reg.m_start >= m_rom_limit) {
				LOG("Region %s beyond ROM limit (start=%04x , limit=%04x)\n" , reg.m_tag , reg.m_start , m_rom_limit);
				return image_init_result::FAIL;
			}
		}
	}

	return image_init_result::PASS;
}

void hp9825_optrom_slot_device::call_unload()
{
	LOG("hp9825_optrom: call_unload\n");
	if (m_space_r != nullptr && m_loaded_regions) {
		unsigned mask = 1;

		for (const struct optrom_region& reg : region_tab) {
			if (m_loaded_regions & mask) {
				if (reg.m_start == 0x5c00) {
					m_space_r->unmap_read(0x3000 , 0x33ff);
					m_space_r->unmap_read(0x5c00 , 0x5fff);
					m_bank->space(AS_PROGRAM).unmap_read(0 , 0x1fff);
				} else {
					m_space_r->unmap_read(reg.m_start , reg.m_start + reg.m_size - 1);
				}
				LOG("%s unloaded\n" , reg.m_tag);
			}
			mask <<= 1;
		}
		m_loaded_regions = 0;
	}
	machine().schedule_soft_reset();
}

std::string hp9825_optrom_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}

