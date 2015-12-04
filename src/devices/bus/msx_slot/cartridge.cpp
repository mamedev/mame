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


const device_type MSX_SLOT_CARTRIDGE = &device_creator<msx_slot_cartridge_device>;
const device_type MSX_SLOT_YAMAHA_EXPANSION = &device_creator<msx_slot_yamaha_expansion_device>;


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_SLOT_CARTRIDGE, "MSX Cartridge slot", tag, owner, clock, "msx_slot_cartridge", __FILE__)
	, device_image_interface(mconfig, *this)
	, device_slot_interface(mconfig, *this)
	, msx_internal_slot_interface()
	, m_irq_handler(*this)
	, m_cartridge(nullptr)
{
}


msx_slot_cartridge_device::msx_slot_cartridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_image_interface(mconfig, *this)
	, device_slot_interface(mconfig, *this)
	, msx_internal_slot_interface()
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
	for (int i = 0; i < ARRAY_LENGTH(slot_list); i++)
	{
		if (slot_list[i].pcb_id == type)
			return slot_list[i].slot_option;
	}

	return slot_list[0].slot_option;
}


void msx_slot_cartridge_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_cartridge = dynamic_cast<msx_cart_interface *>(get_card_device());
}


bool msx_slot_cartridge_device::call_load()
{
	if ( m_cartridge )
	{
		if ( software_entry() )
		{
			UINT32 length;

			// Allocate and copy rom contents
			length = get_software_region_length("rom");
			m_cartridge->rom_alloc( length );
			if (length > 0)
			{
				UINT8 *rom_base = m_cartridge->get_rom_base();
				memcpy(rom_base, get_software_region("rom"), length);
			}

			// Allocate and copy vlm5030 rom contents
			length = get_software_region_length("vlm5030");
			m_cartridge->rom_vlm5030_alloc(length);
			if (length > 0)
			{
				UINT8 *rom_base = m_cartridge->get_rom_vlm5030_base();
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
			UINT32 length = this->length();

			// determine how much space to allocate
			UINT32 length_aligned = 0x10000;

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
				seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read file");
				return IMAGE_INIT_FAIL;
			}

			// Check if there's some mapper related
			std::string extrainfo;
			if (hashfile_extrainfo(*this, extrainfo))
			{
			}
		}

		m_cartridge->set_out_irq_cb(DEVCB_WRITELINE(msx_slot_cartridge_device, irq_out));
		m_cartridge->initialize_cartridge();

		if (m_cartridge->get_sram_size() > 0)
		{
			battery_load(m_cartridge->get_sram_base(), m_cartridge->get_sram_size(), 0x00);
		}
	}
	return IMAGE_INIT_PASS;
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


bool msx_slot_cartridge_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);
	return true;
}


WRITE_LINE_MEMBER(msx_slot_cartridge_device::irq_out)
{
	m_irq_handler(state);
}


int msx_slot_cartridge_device::get_cart_type(UINT8 *rom, UINT32 length)
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

	if (MAX (kon4, kon5) > MAX (asc8, asc16) )
	{
		return (kon5 > kon4) ? KONAMI_SCC : KONAMI;
	}
	else
	{
		return (asc8 > asc16) ? ASCII8 : ASCII16;
	}
}


void msx_slot_cartridge_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "nomapper";
		UINT32 length = core_fsize(m_file);
		dynamic_buffer rom(length);
		int type = NOMAPPER;

		// Check if there's some mapper related information in the hashfiles
		std::string extrainfo;
		if (hashfile_extrainfo(*this, extrainfo))
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

				for (int i = 0; i < ARRAY_LENGTH(extrainfo_map); i++)
				{
					if (extrainfo_map[i].extrainfo == extrainfo_type)
					{
						type = extrainfo_map[i].mapper;
					}
				}
			}
		}

		if (type == NOMAPPER)
		{
			// Not identified through hashfile, try automatic detection
			core_fread(m_file, &rom[0], length);
			type = get_cart_type(&rom[0], length);
		}

		if (type > NOMAPPER)
		{
			slot_string = msx_cart_get_slot_option(type);
		}

		result.assign(slot_string);
		return;
	}
	software_get_default_slot(result, "nomapper");
}


READ8_MEMBER(msx_slot_cartridge_device::read)
{
	if ( m_cartridge )
	{
		return m_cartridge->read_cart(space, offset);
	}
	return 0xFF;
}


WRITE8_MEMBER(msx_slot_cartridge_device::write)
{
	if ( m_cartridge )
	{
		m_cartridge->write_cart(space, offset, data);
	}
}




msx_slot_yamaha_expansion_device::msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_slot_cartridge_device(mconfig, MSX_SLOT_YAMAHA_EXPANSION, "MSX Yamaha Expansion slot", tag, owner, clock, "msx_slot_yamaha_expansion", __FILE__)
{
}


void msx_slot_yamaha_expansion_device::device_start()
{
	m_irq_handler.resolve_safe();
	m_cartridge = dynamic_cast<msx_cart_interface *>(get_card_device());
	if (m_cartridge)
	{
		m_cartridge->set_out_irq_cb(DEVCB_WRITELINE(msx_slot_cartridge_device, irq_out));
	}
}
