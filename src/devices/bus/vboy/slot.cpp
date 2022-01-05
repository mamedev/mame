// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************
 Virtual Boy cartridge slot

 TODO:
 - Sound capabilities
 ***************************************************************************/

#include "emu.h"
#include "slot.h"

#include "emuopts.h"
#include "romload.h"
#include "softlist.h"

#include <cstring>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VBOY_CART_SLOT, vboy_cart_slot_device, "vboy_cart_slot", "Nintendo Virtual Boy Cartridge Slot")


//**************************************************************************
//  vboy_cart_slot_device
//**************************************************************************

vboy_cart_slot_device::vboy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, VBOY_CART_SLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_vboy_cart_interface>(mconfig, *this),
	m_intcro(*this),
	m_exp_space(*this, finder_base::DUMMY_TAG, -1, 32),
	m_chip_space(*this, finder_base::DUMMY_TAG, -1, 32),
	m_rom_space(*this, finder_base::DUMMY_TAG, -1, 32),
	m_exp_base(0U),
	m_chip_base(0U),
	m_rom_base(0U),
	m_cart(nullptr)
{
}


image_init_result vboy_cart_slot_device::call_load()
{
	if (!m_cart)
		return image_init_result::PASS;

	memory_region *romregion(loaded_through_softlist() ? memregion("rom") : nullptr);
	if (loaded_through_softlist() && !romregion)
	{
		seterror(image_error::INVALIDIMAGE, "Software list item has no 'rom' data area");
		return image_init_result::FAIL;
	}

	u32 const len(loaded_through_softlist() ? romregion->bytes() : length());
	if ((0x0000'0003 & len) || (0x0100'0000 < len))
	{
		seterror(image_error::INVALIDIMAGE, "Unsupported cartridge size (must be a multiple of 4 bytes no larger than 16 MiB)");
		return image_init_result::FAIL;
	}

	if (!loaded_through_softlist())
	{
		LOG("Allocating %u byte cartridge ROM region\n", len);
		romregion = machine().memory().region_alloc(subtag("rom").c_str(), len, 4, ENDIANNESS_LITTLE);
		u32 const cnt(fread(romregion->base(), len));
		if (cnt != len)
		{
			seterror(image_error::UNSPECIFIED, "Error reading cartridge file");
			return image_init_result::FAIL;
		}
	}

	return m_cart->load();
}


void vboy_cart_slot_device::call_unload()
{
	if (m_cart)
		m_cart->unload();
}


void vboy_cart_slot_device::device_validity_check(validity_checker &valid) const
{
	if (m_exp_base & 0x00ff'ffff)
		osd_printf_error("EXP base address 0x%X is not on a 24-bit boundary\n", m_exp_base);

	if (m_chip_base & 0x00ff'ffff)
		osd_printf_error("CHIP base address 0x%X is not on a 24-bit boundary\n", m_chip_base);

	if (m_rom_base & 0x00ff'ffff)
		osd_printf_error("ROM base address 0x%X is not on a 24-bit boundary\n", m_rom_base);
}


void vboy_cart_slot_device::device_resolve_objects()
{
	m_intcro.resolve_safe();
}


void vboy_cart_slot_device::device_start()
{
	if (!m_exp_space && ((m_exp_space.finder_tag() != finder_base::DUMMY_TAG) || (m_exp_space.spacenum() >= 0)))
		throw emu_fatalerror("%s: Address space %d of device %s not found (EXP)\n", tag(), m_exp_space.spacenum(), m_exp_space.finder_tag());

	if (!m_chip_space && ((m_chip_space.finder_tag() != finder_base::DUMMY_TAG) || (m_chip_space.spacenum() >= 0)))
		throw emu_fatalerror("%s: Address space %d of device %s not found (CHIP)\n", tag(), m_chip_space.spacenum(), m_chip_space.finder_tag());

	if (!m_rom_space && ((m_rom_space.finder_tag() != finder_base::DUMMY_TAG) || (m_rom_space.spacenum() >= 0)))
		throw emu_fatalerror("%s: Address space %d of device %s not found (ROM)\n", tag(), m_rom_space.spacenum(), m_rom_space.finder_tag());

	m_cart = get_card_device();
}


std::string vboy_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	std::string const image_name(mconfig().options().image_option(instance_name()).value());
	software_part const *const part(!image_name.empty() ? find_software_item(image_name, true) : nullptr);
	if (part)
	{
		//printf("[%s] Found software part for image name '%s'\n", tag(), image_name.c_str());
		for (rom_entry const &entry : part->romdata())
		{
			if (ROMENTRY_ISREGION(entry) && (entry.name() == "sram"))
			{
				//printf("[%s] Found 'sram' data area, enabling cartridge backup RAM\n", tag());
				return "flatrom_sram";
			}
		}
	}
	else
	{
		//printf("[%s] No software part found for image name '%s'\n", tag(), image_name.c_str());
	}

	//printf("[%s] Assuming plain ROM cartridge\n", tag());
	return "flatrom";
}



//**************************************************************************
//  device_vboy_cart_interface
//**************************************************************************

void device_vboy_cart_interface::unload()
{
}


device_vboy_cart_interface::device_vboy_cart_interface(machine_config const &mconfig, device_t &device) :
	device_interface(device, "vboycart"),
	m_slot(dynamic_cast<vboy_cart_slot_device *>(device.owner()))
{
}



#include "rom.h"

void vboy_carts(device_slot_interface &device)
{
	device.option_add_internal("flatrom", VBOY_FLAT_ROM);
	device.option_add_internal("flatrom_sram", VBOY_FLAT_ROM_SRAM);
}
