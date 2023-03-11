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
#include "fs_sr022.h"
#include "halnote.h"
#include "hfox.h"
#include "holy_quran.h"
#include "ink.h"
#include "kanji.h"
#include "konami.h"
#include "korean.h"
#include "majutsushi.h"
#include "moonsound.h"
#include "msx_audio.h"
#include "msxdos2.h"
#include "nomapper.h"
#include "ram.h"
#include "rtype.h"
#include "slotexpander.h"
#include "softcard.h"
#include "superloderunner.h"
#include "super_swangi.h"
#include "yamaha_ucn01.h"

#include "bus/msx/slot/cartridge.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_CARTRIDGE, msx_slot_cartridge_device, "msx_slot_cartridge", "MSX Cartridge slot")


void msx_cart(device_slot_interface &device, bool is_in_subslot)
{
	msx_cart_disk_register_options(device);
	msx_cart_ram_register_options(device);
	device.option_add_internal("arc", MSX_CART_ARC);
	device.option_add_internal("ascii8", MSX_CART_ASCII8);
	device.option_add_internal("ascii8_sram", MSX_CART_ASCII8_SRAM);
	device.option_add_internal("ascii16", MSX_CART_ASCII16);
	device.option_add_internal("ascii16_sram", MSX_CART_ASCII16_SRAM);
	device.option_add_internal("cross_blaim", MSX_CART_CROSSBLAIM);
	device.option_add_internal("dooly", MSX_CART_DOOLY);
	device.option_add_internal("easispeech", MSX_CART_EASISPEECH);
	device.option_add_internal("fmpac", MSX_CART_FMPAC);
	device.option_add_internal("fs_sr022", MSX_CART_FS_SR022);
	device.option_add_internal("gamemaster2", MSX_CART_GAMEMASTER2);
	device.option_add_internal("halnote", MSX_CART_HALNOTE);
	device.option_add_internal("hfox", MSX_CART_HFOX);
	device.option_add_internal("holy_quran", MSX_CART_HOLY_QURAN);
	device.option_add_internal("ink", MSX_CART_INK);
	device.option_add_internal("kanji", MSX_CART_KANJI);
	device.option_add_internal("keyboard_master", MSX_CART_KEYBOARD_MASTER);
	device.option_add_internal("konami", MSX_CART_KONAMI);
	device.option_add_internal("konami_scc", MSX_CART_KONAMI_SCC);
	device.option_add_internal("korean_80in1", MSX_CART_KOREAN_80IN1);
	device.option_add_internal("korean_90in1", MSX_CART_KOREAN_90IN1);
	device.option_add_internal("korean_126in1", MSX_CART_KOREAN_126IN1);
	device.option_add_internal("majutsushi", MSX_CART_MAJUTSUSHI);
	device.option_add_internal("msxaud_fsca1", MSX_CART_MSX_AUDIO_FSCA1);
	device.option_add_internal("msxaud_hxmu900", MSX_CART_MSX_AUDIO_HXMU900);
	device.option_add_internal("msxaud_nms1205", MSX_CART_MSX_AUDIO_NMS1205);
	device.option_add_internal("msxdos2j", MSX_CART_MSXDOS2J);
	device.option_add_internal("msxdos2e", MSX_CART_MSXDOS2E);
	device.option_add_internal("msxwrite", MSX_CART_MSXWRITE);
	device.option_add_internal("nomapper", MSX_CART_NOMAPPER);
	device.option_add_internal("rtype", MSX_CART_RTYPE);
	device.option_add_internal("sound_snatcher", MSX_CART_SOUND_SNATCHER);
	device.option_add_internal("sound_sdsnatch", MSX_CART_SOUND_SDSNATCHER);
	device.option_add_internal("super_swangi", MSX_CART_SUPER_SWANGI);
	device.option_add_internal("superloderunner", MSX_CART_SUPERLODERUNNER);
	device.option_add_internal("synthesizer", MSX_CART_SYNTHESIZER);
	device.option_add_internal("ec701", MSX_CART_EC701);
	device.option_add("beepack", MSX_CART_BEEPACK);
	device.option_add("bm_012", MSX_CART_BM_012);
	device.option_add("moonsound", MSX_CART_MOONSOUND);
	device.option_add("ucn01", MSX_CART_UCN01);
	if (!is_in_subslot)
	{
		device.option_add("slotexp", MSX_CART_SLOTEXPANDER);
	}
	device.option_add("softcard", MSX_CART_SOFTCARD);
}


enum
{
	NOMAPPER = 0,
	ASCII8,
	ASCII8_SRAM,
	ASCII16,
	ASCII16_SRAM,
	CROSSBLAIM,
	GAMEMASTER2,
	KOREAN_80IN1,
	KOREAN_90IN1,
	KOREAN_126IN1,
	FMPAC,
	RTYPE,
	KONAMI,
	KONAMI_SCC,
	SUPERLODERUNNER,
	MAJUTSUSHI,
	DISK_ROM,
	SYNTHESIZER,
	MSXDOS2
};


static const struct
{
	int        pcb_id;
	const char *slot_option;
} slot_list[] =
{
	{ NOMAPPER, "nomapper" },
	{ ASCII8, "ascii8" },
	{ ASCII8_SRAM, "ascii8_sram" },
	{ ASCII16, "ascii16" },
	{ ASCII16_SRAM, "ascii16_sram" },
	{ CROSSBLAIM, "cross_blaim" },
	{ GAMEMASTER2, "gamemaster2" },
	{ KOREAN_80IN1, "korean_80in1" },
	{ KOREAN_90IN1, "korean_90in1" },
	{ KOREAN_126IN1, "korean_126in1" },
	{ FMPAC, "fmpac" },
	{ RTYPE, "rtype" },
	{ KONAMI, "konami" },
	{ KONAMI_SCC, "konami_scc" },
	{ SUPERLODERUNNER, "superloderunner" },
	{ MAJUTSUSHI, "majutsushi" },
	{ DISK_ROM, "disk_rom" },
	{ SYNTHESIZER, "synthesizer" },
	{ MSXDOS2, "msxdos2" }
};


static const char *msx_cart_get_slot_option(int type)
{
	for (auto & elem : slot_list)
	{
		if (elem.pcb_id == type)
			return elem.slot_option;
	}

	return slot_list[0].slot_option;
}



msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_base_device(mconfig, MSX_SLOT_CARTRIDGE, tag, owner, clock)
{
}

std::string msx_slot_cartridge_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		const char *slot_string = "nomapper";
		int type = NOMAPPER;

		// Check if there's some mapper related information in the hashfiles
		std::string extrainfo;
		if (hook.hashfile_extrainfo(extrainfo))
		{
			int extrainfo_type = -1;
			if (1 == sscanf(extrainfo.c_str(), "%d", &extrainfo_type))
			{
				static const struct {int extrainfo; int mapper;} extrainfo_map[] = {
					//{ 0, NOMAPPER },
					{ 1, MSXDOS2 },
					{ 2, KONAMI_SCC },
					{ 3, KONAMI },
					{ 4, ASCII8 },
					{ 5, ASCII16 },
					{ 6, GAMEMASTER2 },
					{ 7, ASCII8_SRAM },
					{ 8, ASCII16_SRAM },
					{ 9, RTYPE },
					{ 10, MAJUTSUSHI },
					{ 11, FMPAC },
					{ 12, SUPERLODERUNNER },
					{ 13, SYNTHESIZER },
					{ 14, CROSSBLAIM },
					{ 15, DISK_ROM },
					{ 16, KOREAN_80IN1 },
					{ 17, KOREAN_126IN1 }
				};

				for (auto & elem : extrainfo_map)
				{
					if (elem.extrainfo == extrainfo_type)
					{
						type = elem.mapper;
					}
				}
			}
		}

		if (type == NOMAPPER)
		{
			// Not identified through hashfile, try automatic detection
			u64 length;
			hook.image_file()->length(length); // FIXME: check error return, guard against excessively large files
			std::vector<u8> rom(length);
			size_t actual;
			hook.image_file()->read(&rom[0], length, actual); // FIXME: check error return or read returning short
			type = get_cart_type(&rom[0], length);
		}

		if (type > NOMAPPER)
		{
			slot_string = msx_cart_get_slot_option(type);
		}

		return std::string(slot_string);
	}
	return software_get_default_slot("nomapper");
}


int msx_slot_cartridge_device::get_cart_type(const u8 *rom, u32 length)
{
	if (length < 0x2000)
	{
		return -1;
	}

	if (length < 0x10000)
	{
		return NOMAPPER;
	}

	if ((rom[0x10] == 'Y') && (rom[0x11] == 'Z') && (length > 0x18000))
	{
		return GAMEMASTER2;
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
		return (kon5 > kon4) ? KONAMI_SCC : KONAMI;
	}
	else
	{
		return (asc8 > asc16) ? ASCII8 : ASCII16;
	}
}
