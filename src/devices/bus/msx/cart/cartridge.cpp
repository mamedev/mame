// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "cartridge.h"

#include "arc.h"
#include "ascii.h"
#include "beepack.h"
#include "bm_012.h"
#include "crossblaim.h"
#include "disk.h"
#include "dooly.h"
#include "easi_speech.h"
#include "fmpac.h"
#include "franky.h"
#include "fs_sr021.h"
#include "fs_sr022.h"
#include "halnote.h"
#include "hbi55.h"
#include "hfox.h"
#include "holy_quran.h"
#include "ide.h"
#include "ink.h"
#include "kanji.h"
#include "konami.h"
#include "korean.h"
#include "loveplus.h"
#include "majutsushi.h"
#include "matra.h"
#include "moonsound.h"
#include "msx_audio.h"
#include "msxdos2.h"
#include "nomapper.h"
#include "quickdisk.h"
#include "ram.h"
#include "rtype.h"
#include "scsi.h"
#include "slotexpander.h"
#include "slotoptions.h"
#include "softcard.h"
#include "super_swangi.h"
#include "superloderunner.h"
#include "yamaha_ucn01.h"

#include "bus/msx/slot/cartridge.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_CARTRIDGE, msx_slot_cartridge_device, "msx_slot_cartridge", "MSX Cartridge slot")


void msx_cart(device_slot_interface &device, bool is_in_subslot)
{
	using namespace bus::msx::cart;
	msx_cart_disk_register_options(device);
	msx_cart_ram_register_options(device);
	device.option_add_internal(slotoptions::ARC,             MSX_CART_ARC);
	device.option_add_internal(slotoptions::ASCII8,          MSX_CART_ASCII8);
	device.option_add_internal(slotoptions::ASCII8_SRAM,     MSX_CART_ASCII8_SRAM);
	device.option_add_internal(slotoptions::ASCII16,         MSX_CART_ASCII16);
	device.option_add_internal(slotoptions::ASCII16_SRAM,    MSX_CART_ASCII16_SRAM);
	device.option_add_internal(slotoptions::CROSS_BLAIM,     MSX_CART_CROSSBLAIM);
	device.option_add_internal(slotoptions::DOOLY,           MSX_CART_DOOLY);
	device.option_add_internal(slotoptions::EASISPEECH,      MSX_CART_EASISPEECH);
	device.option_add_internal(slotoptions::FMPAC,           MSX_CART_FMPAC);
	device.option_add_internal(slotoptions::FS_SR021,        MSX_CART_FS_SR021);
	device.option_add_internal(slotoptions::FS_SR022,        MSX_CART_FS_SR022);
	device.option_add_internal(slotoptions::GAMEMASTER2,     MSX_CART_GAMEMASTER2);
	device.option_add_internal(slotoptions::GOUDA_SCSI,      MSX_CART_GOUDA_SCSI);
	device.option_add_internal(slotoptions::HALNOTE,         MSX_CART_HALNOTE);
	device.option_add_internal(slotoptions::HFOX,            MSX_CART_HFOX);
	device.option_add_internal(slotoptions::HOLY_QURAN,      MSX_CART_HOLY_QURAN);
	device.option_add_internal(slotoptions::INK,             MSX_CART_INK);
	device.option_add_internal(slotoptions::KANJI,           MSX_CART_KANJI);
	device.option_add_internal(slotoptions::KEYBOARD_MASTER, MSX_CART_KEYBOARD_MASTER);
	device.option_add_internal(slotoptions::KONAMI,          MSX_CART_KONAMI);
	device.option_add_internal(slotoptions::KONAMI_SCC,      MSX_CART_KONAMI_SCC);
	device.option_add_internal(slotoptions::KOREAN_25IN1,    MSX_CART_KOREAN_25IN1);
	device.option_add_internal(slotoptions::KOREAN_80IN1,    MSX_CART_KOREAN_80IN1);
	device.option_add_internal(slotoptions::KOREAN_90IN1,    MSX_CART_KOREAN_90IN1);
	device.option_add_internal(slotoptions::KOREAN_126IN1,   MSX_CART_KOREAN_126IN1);
	device.option_add_internal(slotoptions::KOREAN_HYDLIDE2, MSX_CART_KOREAN_HYDLIDE2);
	device.option_add_internal(slotoptions::LOVEPLUS,        MSX_CART_LOVEPLUS);
	device.option_add_internal(slotoptions::MAJUSTUSHI,      MSX_CART_MAJUTSUSHI);
	device.option_add_internal(slotoptions::MATRA_COMP,      MSX_CART_MATRA_COMP);
	device.option_add_internal(slotoptions::MEGA_SCSI,       MSX_CART_MEGA_SCSI);
	device.option_add_internal(slotoptions::MSXAUD_FSCA1,    MSX_CART_MSX_AUDIO_FSCA1);
	device.option_add_internal(slotoptions::MSXAUD_HXMU900,  MSX_CART_MSX_AUDIO_HXMU900);
	device.option_add_internal(slotoptions::MSXAUD_NMS1205,  MSX_CART_MSX_AUDIO_NMS1205);
	device.option_add_internal(slotoptions::MSXDOS2J,        MSX_CART_MSXDOS2J);
	device.option_add_internal(slotoptions::MSXDOS2E,        MSX_CART_MSXDOS2E);
	device.option_add_internal(slotoptions::MSXWRITE,        MSX_CART_MSXWRITE);
	device.option_add_internal(slotoptions::NOMAPPER,        MSX_CART_NOMAPPER);
	device.option_add_internal(slotoptions::QUICKDISK,       MSX_CART_QUICKDISK);
	device.option_add_internal(slotoptions::RTYPE,           MSX_CART_RTYPE);
	device.option_add_internal(slotoptions::SOUND_SNATCHER,  MSX_CART_SOUND_SNATCHER);
	device.option_add_internal(slotoptions::SOUND_SDSNATCH,  MSX_CART_SOUND_SDSNATCHER);
	device.option_add_internal(slotoptions::SUNRISE_SCC,     MSX_CART_SUNRISE_SCC);
	device.option_add_internal(slotoptions::SUPER_SWANGI,    MSX_CART_SUPER_SWANGI);
	device.option_add_internal(slotoptions::SUPERLODERUNNER, MSX_CART_SUPERLODERUNNER);
	device.option_add_internal(slotoptions::SYNTHESIZER,     MSX_CART_SYNTHESIZER);
	device.option_add_internal(slotoptions::EC701,           MSX_CART_EC701);
	device.option_add(slotoptions::BEEPACK,        MSX_CART_BEEPACK);
	device.option_add(slotoptions::BM_012,         MSX_CART_BM_012);
	device.option_add(slotoptions::FRANKY,         MSX_CART_FRANKY);
	device.option_add(slotoptions::HBI55,          MSX_CART_HBI55);
	device.option_add(slotoptions::MOONSOUND,      MSX_CART_MOONSOUND);
	device.option_add(slotoptions::SOFTCARD,       MSX_CART_SOFTCARD);
	device.option_add(slotoptions::SUNRISE_ATAIDE, MSX_CART_SUNRISE_ATAIDE);
	device.option_add(slotoptions::UCN01,          MSX_CART_UCN01);
	if (!is_in_subslot)
	{
		device.option_add(slotoptions::SLOTEXP, MSX_CART_SLOTEXPANDER);
	}
}


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_base_device(mconfig, MSX_SLOT_CARTRIDGE, tag, owner, clock)
{
}

std::string msx_slot_cartridge_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	using namespace bus::msx::cart;
	if (hook.image_file())
	{
		// Check if there's some mapper related information in the hashfiles
		std::string extrainfo;
		if (hook.hashfile_extrainfo(extrainfo))
		{
			int extrainfo_type = -1;
			if (1 == sscanf(extrainfo.c_str(), "%d", &extrainfo_type))
			{
				static const struct {int extrainfo; char const *const mapper;} extrainfo_map[] = {
					{ 0, slotoptions::NOMAPPER },
					{ 1, slotoptions::MSXDOS2E },
					{ 2, slotoptions::KONAMI_SCC },
					{ 3, slotoptions::KONAMI },
					{ 4, slotoptions::ASCII8 },
					{ 5, slotoptions::ASCII16 },
					{ 6, slotoptions::GAMEMASTER2 },
					{ 7, slotoptions::ASCII8_SRAM },
					{ 8, slotoptions::ASCII16_SRAM },
					{ 9, slotoptions::RTYPE },
					{ 10, slotoptions::MAJUSTUSHI },
					{ 11, slotoptions::FMPAC },
					{ 12, slotoptions::SUPERLODERUNNER },
					{ 13, slotoptions::SYNTHESIZER },
					{ 14, slotoptions::CROSS_BLAIM },
					{ 16, slotoptions::KOREAN_80IN1 },
					{ 17, slotoptions::KOREAN_126IN1 }
				};

				for (auto & elem : extrainfo_map)
				{
					if (elem.extrainfo == extrainfo_type)
					{
						return std::string(elem.mapper);
					}
				}
			}
		}

		// Not identified through hashfile, try automatic detection
		u64 length;
		if (hook.image_file()->length(length))
		{
			osd_printf_warning("[%s] Error getting cartridge ROM length\n", tag());
			return std::string(slotoptions::NOMAPPER);
		}
		length = std::min<u64>(length, 4 * 1024 * 1024);
		auto const [err, rom, actual] = read(*hook.image_file(), length);
		if (err || (actual != length))
		{
			osd_printf_warning("[%s] Error reading from file\n", tag());
			return std::string(slotoptions::NOMAPPER);
		}
		return std::string(get_cart_type(&rom[0], length));
	}
	return software_get_default_slot(bus::msx::cart::slotoptions::NOMAPPER);
}


char const *const msx_slot_cartridge_device::get_cart_type(const u8 *rom, u32 length)
{
	using namespace bus::msx::cart;
	if (length < 0x10000)
	{
		return slotoptions::NOMAPPER;
	}

	if ((rom[0x10] == 'Y') && (rom[0x11] == 'Z') && (length > 0x18000))
	{
		return slotoptions::GAMEMASTER2;
	}

	int kon4 = 0, kon5 = 0, asc8 = 0, asc16 = 0;

	for (int i=0; i < length-3; i++)
	{
		if (rom[i] == 0x32 && rom[i+1] == 0)
		{
			switch (rom[i+2])
			{
				case 0x60:
				case 0x70:
					asc16++;
					asc8++;
					break;

				case 0x68:
				case 0x78:
					asc8++;
					asc16--;
					break;
			}

			switch (rom[i+2])
			{
				case 0x60:
				case 0x80:
				case 0xa0:
					kon4++;
					break;

				case 0x50:
				case 0x70:
				case 0x90:
				case 0xb0:
					kon5++;
					break;
			}
		}
	}

	if (std::max(kon4, kon5) > std::max(asc8, asc16))
	{
		return (kon5 > kon4) ? slotoptions::KONAMI_SCC : slotoptions::KONAMI;
	}
	else
	{
		return (asc8 > asc16) ? slotoptions::ASCII8 : slotoptions::ASCII16;
	}
}
