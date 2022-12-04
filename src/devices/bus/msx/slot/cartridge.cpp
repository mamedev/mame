// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "cartridge.h"
#include "hashfile.h"


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


DEFINE_DEVICE_TYPE(MSX_SLOT_CARTRIDGE,        msx_slot_cartridge_device,        "msx_slot_cartridge",        "MSX Cartridge slot")
DEFINE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device, "msx_slot_yamaha_expansion", "MSX Yamaha Expansion slot")


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_CARTRIDGE, tag, owner, clock)
{
}


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_slot_interface(mconfig, *this)
	, msx_internal_slot_interface(mconfig, *this)
	, m_irq_handler(*this)
	, m_cartridge(nullptr)
{
}


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


void msx_slot_cartridge_device::device_resolve_objects()
{
	m_irq_handler.resolve_safe();
	m_cartridge = dynamic_cast<msx_cart_interface *>(get_card_device());
	if (m_cartridge)
	{
		m_cartridge->set_views(page(0), page(1), page(2), page(3));
	}
}


void msx_slot_cartridge_device::device_start()
{
}


image_init_result msx_slot_cartridge_device::call_load()
{
	if (m_cartridge)
	{
		if (!loaded_through_softlist())
		{
			u32 length = this->length();

			// determine how much space to allocate
			u32 length_aligned = 0x10000;

			if (length <= 0x2000)
				length_aligned = 0x2000;
			else if (length <= 0x4000)
				length_aligned = 0x4000;
			else if (length <= 0x8000)
				length_aligned = 0x8000;
			else if (length <= 0xc000)
				length_aligned = 0xc000;
			else
			{
				while (length_aligned < length)
					length_aligned *= 2;
			}

			memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length_aligned, 1, ENDIANNESS_LITTLE);
			if (fread(romregion->base(), length) != length)
			{
				seterror(image_error::UNSPECIFIED, "Unable to fully read file");
				return image_init_result::FAIL;
			}

			// Check if there's some mapper related
			std::string extrainfo;
			if (hashfile_extrainfo(*this, extrainfo))
			{
			}
		}

		std::string message;
		image_init_result result = m_cartridge->initialize_cartridge(message);
		if (image_init_result::PASS != result)
		{
			seterror(image_error::INVALIDIMAGE, message.c_str());
			return result;
		}

		if (m_cartridge->cart_sram_region())
		{
			battery_load(m_cartridge->cart_sram_region()->base(), m_cartridge->cart_sram_region()->bytes(), 0x00);
		}
	}
	return image_init_result::PASS;
}


void msx_slot_cartridge_device::call_unload()
{
	if (m_cartridge && m_cartridge->cart_sram_region())
	{
		battery_save(m_cartridge->cart_sram_region()->base(), m_cartridge->cart_sram_region()->bytes());
	}
}


WRITE_LINE_MEMBER(msx_slot_cartridge_device::irq_out)
{
	m_irq_handler(state);
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




msx_slot_yamaha_expansion_device::msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_YAMAHA_EXPANSION, tag, owner, clock)
{
}

void msx_slot_yamaha_expansion_device::device_start()
{
}


msx_cart_interface::msx_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "msxcart")
	, m_exp(dynamic_cast<msx_slot_cartridge_device *>(device.owner()))
{
	for (int i = 0; i < 4; i++)
		m_page[i] = nullptr;
}

WRITE_LINE_MEMBER(msx_cart_interface::irq_out)
{
	m_exp->irq_out(state);
}

address_space &msx_cart_interface::memory_space() const
{
	return m_exp->memory_space();
}

address_space &msx_cart_interface::io_space() const
{
	return m_exp->io_space();
}

cpu_device &msx_cart_interface::maincpu() const
{
	return m_exp->maincpu();
}

void msx_cart_interface::set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3)
{
	m_page[0] = page0;
	m_page[1] = page1;
	m_page[2] = page2;
	m_page[3] = page3;
}
