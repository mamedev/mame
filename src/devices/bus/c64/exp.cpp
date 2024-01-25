// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"

#include "formats/cbm_crt.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_EXPANSION_SLOT, c64_expansion_slot_device, "c64_expansion_slot", "C64 expansion port")



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_c64_expansion_card_interface - constructor
//-------------------------------------------------

device_c64_expansion_card_interface::device_c64_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "c64exp"),
	m_roml_size(0),
	m_romh_size(0),
	m_game(1),
	m_exrom(1)
{
	m_slot = dynamic_cast<c64_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_c64_expansion_card_interface - destructor
//-------------------------------------------------

device_c64_expansion_card_interface::~device_c64_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_expansion_slot_device - constructor
//-------------------------------------------------

c64_expansion_slot_device::c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_c64_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_read_dma_cd(*this, 0),
	m_write_dma_cd(*this),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_dma(*this),
	m_write_reset(*this), m_card(nullptr), m_hiram(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_expansion_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_expansion_slot_device::device_reset()
{
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> c64_expansion_slot_device::call_load()
{
	std::error_condition err;

	if (m_card)
	{
		m_card->m_roml_size = 0;
		m_card->m_romh_size = 0;
		m_card->m_exrom = 1;
		m_card->m_game = 1;

		size_t size;

		if (!loaded_through_softlist())
		{
			size = length();

			if (is_filetype("80"))
			{
				fread(m_card->m_roml, size);
				m_card->m_roml_size = size;
				m_card->m_exrom = 0;

				if (size == 0x4000)
				{
					m_card->m_game = 0;
				}
			}
			else if (is_filetype("a0"))
			{
				fread(m_card->m_romh, 0x2000);
				m_card->m_romh_size = 0x2000;

				m_card->m_exrom = 0;
				m_card->m_game = 0;
			}
			else if (is_filetype("e0"))
			{
				fread(m_card->m_romh, 0x2000);
				m_card->m_romh_size = 0x2000;

				m_card->m_game = 0;
			}
			else if (is_filetype("crt"))
			{
				if (cbm_crt_read_header(image_core_file(), &m_card->m_roml_size, &m_card->m_romh_size, &m_card->m_exrom, &m_card->m_game))
				{
					uint8_t *roml = nullptr;
					uint8_t *romh = nullptr;

					m_card->m_roml = std::make_unique<uint8_t[]>(m_card->m_roml_size);
					m_card->m_romh = std::make_unique<uint8_t[]>(m_card->m_romh_size);

					if (m_card->m_roml_size) roml = m_card->m_roml.get();
					if (m_card->m_romh_size) romh = m_card->m_romh.get();

					cbm_crt_read_data(image_core_file(), roml, romh);
				}
			}
			else
			{
				err = image_error::INVALIDIMAGE;
			}
		}
		else
		{
			size = get_software_region_length("uprom");

			if (size)
			{
				// Ultimax (VIC-10) cartridge
				load_software_region("lorom", m_card->m_roml);
				load_software_region("uprom", m_card->m_romh);
				m_card->m_roml_size = get_software_region_length("lorom");
				m_card->m_romh_size = get_software_region_length("uprom");

				m_card->m_exrom = 1;
				m_card->m_game = 0;
			}
			else
			{
				// Commodore 64/128 cartridge
				load_software_region("roml", m_card->m_roml);
				load_software_region("romh", m_card->m_romh);
				load_software_region("romx", m_card->m_romx);
				load_software_region("nvram", m_card->m_nvram);
				m_card->m_roml_size = get_software_region_length("roml");
				m_card->m_romh_size = get_software_region_length("romh");

				if (get_feature("exrom") != nullptr) m_card->m_exrom = atol(get_feature("exrom"));
				if (get_feature("game") != nullptr) m_card->m_game = atol(get_feature("game"));
			}
		}

		if ((m_card->m_roml_size & (m_card->m_roml_size - 1)) || (m_card->m_romh_size & (m_card->m_romh_size - 1)))
			return std::make_pair(image_error::INVALIDLENGTH, "ROM size must be power of 2");
	}

	return std::make_pair(err, std::string());
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string c64_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		if (hook.is_filetype("crt"))
			return cbm_crt_get_card(*hook.image_file());
	}

	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_expansion_slot_device::cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_card != nullptr)
	{
		data = m_card->c64_cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void c64_expansion_slot_device::cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_card != nullptr)
	{
		m_card->c64_cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
	}
}


//-------------------------------------------------
//  game_r - GAME read
//-------------------------------------------------

int c64_expansion_slot_device::game_r(offs_t offset, int sphi2, int ba, int rw, int loram, int hiram)
{
	int state = 1;

	m_hiram = hiram;
	m_loram = loram;

	if (m_card != nullptr)
	{
		state = m_card->c64_game_r(offset, sphi2, ba, rw);
	}

	return state;
}


//-------------------------------------------------
//  exrom_r - EXROM read
//-------------------------------------------------

int c64_expansion_slot_device::exrom_r(offs_t offset, int sphi2, int ba, int rw, int loram, int hiram)
{
	int state = 1;

	m_hiram = hiram;
	m_loram = loram;

	if (m_card != nullptr)
	{
		state = m_card->c64_exrom_r(offset, sphi2, ba, rw);
	}

	return state;
}


void c64_expansion_slot_device::set_passthrough()
{
	irq_callback().set(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::irq_w));
	nmi_callback().set(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::nmi_w));
	reset_callback().set(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w));
	cd_input_callback().set(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::dma_cd_r));
	cd_output_callback().set(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::dma_cd_w));
	dma_callback().set(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::dma_w));
}

//-------------------------------------------------
//  SLOT_INTERFACE( c64_expansion_cards )
//-------------------------------------------------


// slot devices
#include "16kb.h"
#include "buscard.h"
#include "buscard2.h"
#include "c128_comal80.h"
#include "c128_partner.h"
#include "comal80.h"
#include "cpm.h"
#include "currah_speech.h"
#include "dela_ep256.h"
#include "dela_ep64.h"
#include "dela_ep7x8.h"
#include "dinamic.h"
#include "dqbb.h"
#include "easy_calc_result.h"
#include "easyflash.h"
#include "epyx_fast_load.h"
#include "exos.h"
#include "fcc.h"
#include "final.h"
#include "final3.h"
#include "fun_play.h"
#include "georam.h"
#include "ide64.h"
#include "ieee488.h"
#include "kingsoft.h"
#include "mach5.h"
#include "magic_desk.h"
#include "magic_formel.h"
#include "magic_voice.h"
#include "midi_maplin.h"
#include "midi_namesoft.h"
#include "midi_passport.h"
#include "midi_sci.h"
#include "midi_siel.h"
#include "mikro_assembler.h"
#include "multiscreen.h"
#include "music64.h"
#include "neoram.h"
#include "ocean.h"
#include "pagefox.h"
#include "partner.h"
#include "prophet64.h"
#include "ps64.h"
#include "reu.h"
#include "rex.h"
#include "rex_ep256.h"
#include "ross.h"
#include "sfx_sound_expander.h"
#include "silverrock.h"
#include "simons_basic.h"
#include "speakeasy.h"
#include "stardos.h"
#include "std.h"
#include "structured_basic.h"
#include "super_explode.h"
#include "super_games.h"
#include "supercpu.h"
#include "sw8k.h"
#include "swiftlink.h"
#include "system3.h"
#include "tibdd001.h"
#include "tdos.h"
#include "turbo232.h"
#include "vizastar.h"
#include "vw64.h"
#include "warp_speed.h"
#include "westermann.h"
#include "xl80.h"
#include "zaxxon.h"
#include "z80videopak.h"

void c64_expansion_cards(device_slot_interface &device)
{
	device.option_add("16k", C64_16KB);
	device.option_add("cpm", C64_CPM);
	device.option_add("dqbb", C64_DQBB);
	device.option_add("easyflash", C64_EASYFLASH);
	device.option_add("georam", C64_GEORAM);
	device.option_add("ide64", C64_IDE64);
	device.option_add("midimap", C64_MIDI_MAPLIN);
	device.option_add("midins", C64_MIDI_NAMESOFT);
	device.option_add("midipp", C64_MIDI_PASSPORT);
	device.option_add("midisci", C64_MIDI_SCI);
	device.option_add("midisiel", C64_MIDI_SIEL);
	device.option_add("music64", C64_MUSIC64);
	device.option_add("neoram", C64_NEORAM);
	device.option_add("reu1700", C64_REU1700);
	device.option_add("reu1750", C64_REU1750);
	device.option_add("reu1764", C64_REU1764);
	device.option_add("sfxse", C64_SFX_SOUND_EXPANDER);
	device.option_add("speakez", C64_SPEAKEASY);
	device.option_add("supercpu", C64_SUPERCPU);
	device.option_add("swiftlink", C64_SWIFTLINK);
	device.option_add("tibdd001", C64_TIB_DD_001);
	device.option_add("turbo232", C64_TURBO232);
	device.option_add("buscard", C64_BUSCARD);
	device.option_add("buscard2", C64_BUSCARD2);

	// the following need ROMs from the software list
	device.option_add_internal("standard", C64_STD);
	device.option_add_internal("comal80", C64_COMAL80);
	device.option_add_internal("c128_comal80", C128_COMAL80);
	device.option_add_internal("cs64", C64_CURRAH_SPEECH);
	device.option_add_internal("dela_ep256", C64_DELA_EP256);
	device.option_add_internal("ep64", C64_DELA_EP64);
	device.option_add_internal("ep7x8", C64_DELA_EP7X8);
	device.option_add_internal("dinamic", C64_DINAMIC);
	device.option_add_internal("easycalcres", C64_EASY_CALC_RESULT);
	device.option_add_internal("epyxfastload", C64_EPYX_FAST_LOAD);
	device.option_add_internal("exos", C64_EXOS);
	device.option_add_internal("fcc", C64_FCC);
	device.option_add_internal("final", C64_FINAL);
	device.option_add_internal("final3", C64_FINAL3);
	device.option_add_internal("fun_play", C64_FUN_PLAY);
	device.option_add_internal("ieee488", C64_IEEE488);
	device.option_add_internal("kingsoft", C64_KINGSOFT);
	device.option_add_internal("mach5", C64_MACH5);
	device.option_add_internal("magic_desk", C64_MAGIC_DESK);
	device.option_add_internal("magic_formel", C64_MAGIC_FORMEL);
	device.option_add_internal("magic_voice", C64_MAGIC_VOICE);
	device.option_add_internal("mikroasm", C64_MIKRO_ASSEMBLER);
	device.option_add_internal("multiscreen", C64_MULTISCREEN);
	device.option_add_internal("ocean", C64_OCEAN);
	device.option_add_internal("pagefox", C64_PAGEFOX);
	device.option_add_internal("partner", C64_PARTNER);
	device.option_add_internal("partner128", C128_PARTNER);
	device.option_add_internal("prophet64", C64_PROPHET64);
	device.option_add_internal("ps64", C64_PS64);
	device.option_add_internal("rex", C64_REX);
	device.option_add_internal("rex_ep256", C64_REX_EP256);
	device.option_add_internal("ross", C64_ROSS);
	device.option_add_internal("silverrock", C64_SILVERROCK);
	device.option_add_internal("simons_basic", C64_SIMONS_BASIC);
	device.option_add_internal("stardos", C64_STARDOS);
	device.option_add_internal("struct_basic", C64_STRUCTURED_BASIC);
	device.option_add_internal("super_explode", C64_SUPER_EXPLODE);
	device.option_add_internal("super_games", C64_SUPER_GAMES);
	device.option_add_internal("sw8k", C64_SW8K);
	device.option_add_internal("system3", C64_SYSTEM3);
	device.option_add_internal("tdos", C64_TDOS);
	device.option_add_internal("vizastar", C64_VIZASTAR);
	device.option_add_internal("vizawrite", C64_VW64);
	device.option_add_internal("warp_speed", C64_WARP_SPEED);
	device.option_add_internal("westermann", C64_WESTERMANN);
	device.option_add_internal("zaxxon", C64_ZAXXON);
	device.option_add_internal("xl80", C64_XL80);
	device.option_add_internal("z80videopak", C64_Z80VIDEOPAK);
}
