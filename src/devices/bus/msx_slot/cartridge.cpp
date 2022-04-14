// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "bus/msx_slot/cartridge.h"
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


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_CARTRIDGE, tag, owner, clock)
{
}


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
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
		m_cartridge->m_exp = this;
}


void msx_slot_cartridge_device::device_start()
{
}


image_init_result msx_slot_cartridge_device::call_load()
{
	if ( m_cartridge )
	{
		if (loaded_through_softlist())
		{
			uint32_t length;

			// Allocate and copy rom contents
			length = get_software_region_length("rom");
			m_cartridge->rom_alloc( length );
			if (length > 0)
			{
				uint8_t *rom_base = m_cartridge->get_rom_base();
				memcpy(rom_base, get_software_region("rom"), length);
			}

			// Allocate and copy vlm5030 rom contents
			length = get_software_region_length("vlm5030");
			m_cartridge->rom_vlm5030_alloc(length);
			if (length > 0)
			{
				uint8_t *rom_base = m_cartridge->get_rom_vlm5030_base();
				memcpy(rom_base, get_software_region("vlm5030"), length);
			}

			// Allocate ram
			length = get_software_region_length("ram");
			m_cartridge->ram_alloc( length );

			// Allocate sram
			length = get_software_region_length("sram");
			m_cartridge->sram_alloc( length );
		}
		else
		{
			uint32_t length = this->length();

			// determine how much space to allocate
			uint32_t length_aligned = 0x10000;

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
				while (length_aligned < length )
					length_aligned *= 2;
			}

			m_cartridge->rom_alloc(length_aligned);
			m_cartridge->ram_alloc(0);
			m_cartridge->sram_alloc(0);

			if (fread(m_cartridge->get_rom_base(), length) != length)
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

		m_cartridge->m_exp = this;
		m_cartridge->initialize_cartridge();

		if (m_cartridge->get_sram_size() > 0)
		{
			battery_load(m_cartridge->get_sram_base(), m_cartridge->get_sram_size(), 0x00);
		}
	}
	return image_init_result::PASS;
}


void msx_slot_cartridge_device::call_unload()
{
	if (m_cartridge)
	{
		if (m_cartridge->get_sram_size() > 0)
		{
			battery_save(m_cartridge->get_sram_base(), m_cartridge->get_sram_size());
		}
	}
}


WRITE_LINE_MEMBER(msx_slot_cartridge_device::irq_out)
{
	m_irq_handler(state);
}


int msx_slot_cartridge_device::get_cart_type(const uint8_t *rom, uint32_t length)
{
	if (length < 0x2000)
	{
		return -1;
	}

	if (length < 0x10000)
	{
		return NOMAPPER;
	}

	if ( (rom[0x10] == 'Y') && (rom[0x11] == 'Z') && (length > 0x18000) )
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
				static const struct { int extrainfo; int mapper; } extrainfo_map[] = {
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
			uint64_t length;
			hook.image_file()->length(length); // FIXME: check error return, guard against excessively large files
			std::vector<uint8_t> rom(length);
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


uint8_t msx_slot_cartridge_device::read(offs_t offset)
{
	if ( m_cartridge )
	{
		return m_cartridge->read_cart(offset);
	}
	return 0xFF;
}


void msx_slot_cartridge_device::write(offs_t offset, uint8_t data)
{
	if ( m_cartridge )
	{
		m_cartridge->write_cart(offset, data);
	}
}




msx_slot_yamaha_expansion_device::msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_YAMAHA_EXPANSION, tag, owner, clock)
{
}


void msx_slot_yamaha_expansion_device::device_start()
{
}
