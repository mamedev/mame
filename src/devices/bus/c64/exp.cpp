// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 Expansion Port emulation

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_EXPANSION_SLOT = &device_creator<c64_expansion_slot_device>;



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_c64_expansion_card_interface - constructor
//-------------------------------------------------

device_c64_expansion_card_interface::device_c64_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_roml(*this, "roml"),
		m_romh(*this, "romh"),
		m_nvram(*this, "nvram"),
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

c64_expansion_slot_device::c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, C64_EXPANSION_SLOT, "C64 expansion port", tag, owner, clock, "c64_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this),
		m_read_dma_cd(*this),
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
	m_card = dynamic_cast<device_c64_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_read_dma_cd.resolve_safe(0);
	m_write_dma_cd.resolve_safe();
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_dma.resolve_safe();
	m_write_reset.resolve_safe();

	// inherit bus clock
	if (clock() == 0)
	{
		c64_expansion_slot_device *root = machine().device<c64_expansion_slot_device>(C64_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_expansion_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool c64_expansion_slot_device::call_load()
{
	if (m_card)
	{
		size_t size = 0;

		if (software_entry() == nullptr)
		{
			size = length();

			if (!core_stricmp(filetype(), "80"))
			{
				fread(m_card->m_roml, size);
				m_card->m_exrom = (0);

				if (size == 0x4000)
				{
					m_card->m_game = 0;
				}
			}
			else if (!core_stricmp(filetype(), "a0"))
			{
				fread(m_card->m_romh, 0x2000);

				m_card->m_exrom = 0;
				m_card->m_game = 0;
			}
			else if (!core_stricmp(filetype(), "e0"))
			{
				fread(m_card->m_romh, 0x2000);

				m_card->m_game = 0;
			}
			else if (!core_stricmp(filetype(), "crt"))
			{
				size_t roml_size = 0;
				size_t romh_size = 0;
				int exrom = 1;
				int game = 1;

				if (cbm_crt_read_header(m_file, &roml_size, &romh_size, &exrom, &game))
				{
					UINT8 *roml = nullptr;
					UINT8 *romh = nullptr;

					m_card->m_roml.allocate(roml_size);
					m_card->m_romh.allocate(romh_size);

					if (roml_size) roml = m_card->m_roml;
					if (romh_size) romh = m_card->m_roml;

					cbm_crt_read_data(m_file, roml, romh);
				}

				m_card->m_exrom = exrom;
				m_card->m_game = game;
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

				m_card->m_exrom = 1;
				m_card->m_game = 0;
			}
			else
			{
				// Commodore 64/128 cartridge
				load_software_region("roml", m_card->m_roml);
				load_software_region("romh", m_card->m_romh);
				load_software_region("nvram", m_card->m_nvram);

				if (get_feature("exrom") != nullptr) m_card->m_exrom = atol(get_feature("exrom"));
				if (get_feature("game") != nullptr) m_card->m_game = atol(get_feature("game"));
			}
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool c64_expansion_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

void c64_expansion_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		if (!core_stricmp(filetype(), "crt"))
		{
			cbm_crt_get_card(result, m_file);
			return;
		}

		clear();
	}

	software_get_default_slot(result, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_card != nullptr)
	{
		data = m_card->c64_cd_r(space, offset, data, sphi2, ba, roml, romh, io1, io2);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void c64_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_card != nullptr)
	{
		m_card->c64_cd_w(space, offset, data, sphi2, ba, roml, romh, io1, io2);
	}
}


//-------------------------------------------------
//  game_r - GAME read
//-------------------------------------------------

int c64_expansion_slot_device::game_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	int state = 1;

	m_hiram = hiram;

	if (m_card != nullptr)
	{
		state = m_card->c64_game_r(offset, sphi2, ba, rw);
	}

	return state;
}


//-------------------------------------------------
//  exrom_r - EXROM read
//-------------------------------------------------

int c64_expansion_slot_device::exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram)
{
	int state = 1;

	m_hiram = hiram;

	if (m_card != nullptr)
	{
		state = m_card->c64_exrom_r(offset, sphi2, ba, rw);
	}

	return state;
}


//-------------------------------------------------
//  SLOT_INTERFACE( c64_expansion_cards )
//-------------------------------------------------


// slot devices
#include "16kb.h"
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
#include "stardos.h"
#include "std.h"
#include "structured_basic.h"
#include "super_explode.h"
#include "super_games.h"
#include "supercpu.h"
#include "sw8k.h"
#include "swiftlink.h"
#include "system3.h"
#include "tdos.h"
#include "turbo232.h"
#include "vizastar.h"
#include "vw64.h"
#include "warp_speed.h"
#include "westermann.h"
#include "xl80.h"
#include "zaxxon.h"

SLOT_INTERFACE_START( c64_expansion_cards )
	SLOT_INTERFACE("16k", C64_16KB)
	SLOT_INTERFACE("cpm", C64_CPM)
	SLOT_INTERFACE("dqbb", C64_DQBB)
	SLOT_INTERFACE("easyflash", C64_EASYFLASH)
	SLOT_INTERFACE("georam", C64_GEORAM)
	SLOT_INTERFACE("ide64", C64_IDE64)
	SLOT_INTERFACE("midimap", C64_MIDI_MAPLIN)
	SLOT_INTERFACE("midins", C64_MIDI_NAMESOFT)
	SLOT_INTERFACE("midipp", C64_MIDI_PASSPORT)
	SLOT_INTERFACE("midisci", C64_MIDI_SCI)
	SLOT_INTERFACE("midisiel", C64_MIDI_SIEL)
	SLOT_INTERFACE("music64", C64_MUSIC64)
	SLOT_INTERFACE("neoram", C64_NEORAM)
	SLOT_INTERFACE("reu1700", C64_REU1700)
	SLOT_INTERFACE("reu1750", C64_REU1750)
	SLOT_INTERFACE("reu1764", C64_REU1764)
	SLOT_INTERFACE("sfxse", C64_SFX_SOUND_EXPANDER)
	SLOT_INTERFACE("supercpu", C64_SUPERCPU)
	SLOT_INTERFACE("swiftlink", C64_SWIFTLINK)
	SLOT_INTERFACE("turbo232", C64_TURBO232)

	// the following need ROMs from the software list
	SLOT_INTERFACE_INTERNAL("standard", C64_STD)
	SLOT_INTERFACE_INTERNAL("comal80", C64_COMAL80)
	SLOT_INTERFACE_INTERNAL("c128_comal80", C128_COMAL80)
	SLOT_INTERFACE_INTERNAL("cs64", C64_CURRAH_SPEECH)
	SLOT_INTERFACE_INTERNAL("dela_ep256", C64_DELA_EP256)
	SLOT_INTERFACE_INTERNAL("ep64", C64_DELA_EP64)
	SLOT_INTERFACE_INTERNAL("ep7x8", C64_DELA_EP7X8)
	SLOT_INTERFACE_INTERNAL("dinamic", C64_DINAMIC)
	SLOT_INTERFACE_INTERNAL("easycalcres", C64_EASY_CALC_RESULT)
	SLOT_INTERFACE_INTERNAL("epyxfastload", C64_EPYX_FAST_LOAD)
	SLOT_INTERFACE_INTERNAL("exos", C64_EXOS)
	SLOT_INTERFACE_INTERNAL("fcc", C64_FCC)
	SLOT_INTERFACE_INTERNAL("final", C64_FINAL)
	SLOT_INTERFACE_INTERNAL("final3", C64_FINAL3)
	SLOT_INTERFACE_INTERNAL("fun_play", C64_FUN_PLAY)
	SLOT_INTERFACE_INTERNAL("ieee488", C64_IEEE488)
	SLOT_INTERFACE_INTERNAL("kingsoft", C64_KINGSOFT)
	SLOT_INTERFACE_INTERNAL("mach5", C64_MACH5)
	SLOT_INTERFACE_INTERNAL("magic_desk", C64_MAGIC_DESK)
	SLOT_INTERFACE_INTERNAL("magic_formel", C64_MAGIC_FORMEL)
	SLOT_INTERFACE_INTERNAL("magic_voice", C64_MAGIC_VOICE)
	SLOT_INTERFACE_INTERNAL("mikroasm", C64_MIKRO_ASSEMBLER)
	SLOT_INTERFACE_INTERNAL("multiscreen", C64_MULTISCREEN)
	SLOT_INTERFACE_INTERNAL("ocean", C64_OCEAN)
	SLOT_INTERFACE_INTERNAL("pagefox", C64_PAGEFOX)
	SLOT_INTERFACE_INTERNAL("partner", C64_PARTNER)
	SLOT_INTERFACE_INTERNAL("partner128", C128_PARTNER)
	SLOT_INTERFACE_INTERNAL("prophet64", C64_PROPHET64)
	SLOT_INTERFACE_INTERNAL("ps64", C64_PS64)
	SLOT_INTERFACE_INTERNAL("rex", C64_REX)
	SLOT_INTERFACE_INTERNAL("rex_ep256", C64_REX_EP256)
	SLOT_INTERFACE_INTERNAL("ross", C64_ROSS)
	SLOT_INTERFACE_INTERNAL("silverrock", C64_SILVERROCK)
	SLOT_INTERFACE_INTERNAL("simons_basic", C64_SIMONS_BASIC)
	SLOT_INTERFACE_INTERNAL("stardos", C64_STARDOS)
	SLOT_INTERFACE_INTERNAL("struct_basic", C64_STRUCTURED_BASIC)
	SLOT_INTERFACE_INTERNAL("super_explode", C64_SUPER_EXPLODE)
	SLOT_INTERFACE_INTERNAL("super_games", C64_SUPER_GAMES)
	SLOT_INTERFACE_INTERNAL("sw8k", C64_SW8K)
	SLOT_INTERFACE_INTERNAL("system3", C64_SYSTEM3)
	SLOT_INTERFACE_INTERNAL("tdos", C64_TDOS)
	SLOT_INTERFACE_INTERNAL("vizastar", C64_VIZASTAR)
	SLOT_INTERFACE_INTERNAL("vizawrite", C64_VW64)
	SLOT_INTERFACE_INTERNAL("warp_speed", C64_WARP_SPEED)
	SLOT_INTERFACE_INTERNAL("westermann", C64_WESTERMANN)
	SLOT_INTERFACE_INTERNAL("zaxxon", C64_ZAXXON)
	SLOT_INTERFACE_INTERNAL("xl80", C64_XL80)
SLOT_INTERFACE_END
