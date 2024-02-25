// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Mega Duck cartridge slot

 ***************************************************************************/

#include "emu.h"
#include "mdslot.h"

#include "carts.h"

#include "emuopts.h"
#include "romload.h"
#include "softlist.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


//**************************************************************************
//  megaduck_cart_slot_device
//**************************************************************************

megaduck_cart_slot_device::megaduck_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	gb_cart_slot_device_base(mconfig, MEGADUCK_CART_SLOT, tag, owner, clock)
{
}


std::string megaduck_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	using namespace bus::gameboy;

	if (hook.image_file())
	{
		// loading from file - guess banking scheme using heuristics
		u64 length;
		if (!hook.image_file()->length(length))
		{
			if (0x8000 < length)
			{
				osd_printf_verbose("[%s] 0x%X-byte cartridge, requires ROM banking\n", tag(), length);
				return slotoptions::MEGADUCK_BANKED;
			}
			else
			{
				osd_printf_verbose("[%s] 0x%X-byte cartridge, assuming flat ROM\n", tag(), length);
				return slotoptions::MEGADUCK_STD;
			}
		}
		else
		{
			osd_printf_warning("[%s] Error getting cartridge ROM length - assuming banked ROM\n", tag());
			return slotoptions::MEGADUCK_BANKED;
		}
	}
	else
	{
		// loading from software list - try to find matching software part
		std::string const image_name(mconfig().options().image_option(instance_name()).value());
		software_part const *const part(!image_name.empty() ? find_software_item(image_name, true) : nullptr);
		if (part)
		{
			osd_printf_verbose("[%s] Found software part for image name '%s'\n", tag(), image_name);

			// if there's an explicit "slot" feature, use it
			char const *const slot(part->feature("slot"));
			if (slot)
			{
				osd_printf_verbose("[%s] Using specified cartridge device '%s'\n", tag(), slot);
				return slot;
			}

			// presence or absence of a fixed bank implies banking is in use
			char const *const fixedbank(part->feature("fixedbank"));
			if (fixedbank)
			{
				osd_printf_verbose("[%s] fixedbank='%s' specified, assuming banked ROM\n", tag(), fixedbank);
				return slotoptions::MEGADUCK_BANKED;
			}

			// fall back to guessing based on length
			for (rom_entry const &entry : part->romdata())
			{
				if (ROMENTRY_ISREGION(entry) && (entry.name() == "rom"))
				{
					auto const length(ROMREGION_GETLENGTH(entry));
					if (0x8000 < length)
					{
						osd_printf_verbose("[%s] Found 0x%X-byte 'rom' region, requires ROM banking\n", tag(), length);
						return slotoptions::MEGADUCK_BANKED;
					}
					else
					{
						osd_printf_verbose("[%s] Found 0x%X-byte 'rom' region, assuming flat ROM\n", tag(), length);
						return slotoptions::MEGADUCK_STD;
					}
				}
			}

			// a flat ROM cartridge with nothing in it is harmless
			osd_printf_verbose("[%s] No ROM region found\n", tag());
			return slotoptions::MEGADUCK_STD;
		}
		else
		{
			osd_printf_verbose("[%s] No software part found for image name '%s'\n", tag(), image_name);
		}
	}

	// leave the slot empty
	return std::string();
}


std::pair<std::error_condition, std::string> megaduck_cart_slot_device::load_image_file(util::random_read &file)
{
	auto const len = length();

	if (len)
	{
		LOG("Allocating %u byte cartridge ROM region\n", len);
		memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), len, 1, ENDIANNESS_LITTLE);
		auto const [err, actual] = read_at(file, 0, romregion->base(), len);
		if (err || (len != actual))
			std::make_pair(err ? err : std::errc::io_error, "Error reading cartridge file");
	}

	return std::make_pair(std::error_condition(), std::string());
}



//**************************************************************************
//  Device type definitions
//**************************************************************************

DEFINE_DEVICE_TYPE(MEGADUCK_CART_SLOT, megaduck_cart_slot_device, "megaduck_cart_slot", "Mega Duck Cartridge Slot")
