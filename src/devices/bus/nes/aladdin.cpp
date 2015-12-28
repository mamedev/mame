// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Camerica/Codemasters Aladdin Deck Enhancer pass-through cart


 Here we emulate the following PCBs

 * Camerica ALGNV11 [mapper 71]
 * Camerica ALGQV11 [mapper 232]

 ***********************************************************************************************************/


#include "emu.h"
#include "aladdin.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)



//----------------------------------
//
//  Aladdin Cartslot implementation
//
//----------------------------------

//-------------------------------------------------
//  sub-cart interface
//-------------------------------------------------

aladdin_cart_interface::aladdin_cart_interface(const machine_config &mconfig, device_t &device)
						: device_slot_card_interface(mconfig, device),
							m_rom(nullptr),
							m_rom_size(0),
	m_lobank(0),
	m_hibank(0),
							m_rom_mask(0xff)
{
}

aladdin_cart_interface::~aladdin_cart_interface()
{
}

READ8_MEMBER(aladdin_cart_interface::read)
{
	if (offset < 0x4000)
		return m_rom[(m_lobank * 0x4000) + (offset & 0x3fff)];
	else
		return m_rom[(m_hibank * 0x4000) + (offset & 0x3fff)];
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

const device_type NES_ALADDIN_SLOT = &device_creator<nes_aladdin_slot_device>;

nes_aladdin_slot_device::nes_aladdin_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NES_ALADDIN_SLOT, "NES Aladdin Deck Enhancer Cartridge Slot", tag, owner, clock, "nes_ade_slot", __FILE__),
						device_image_interface(mconfig, *this),
						device_slot_interface(mconfig, *this), m_cart(nullptr)
{
}

nes_aladdin_slot_device::~nes_aladdin_slot_device()
{
}


void nes_aladdin_slot_device::device_start()
{
	m_cart = dynamic_cast<aladdin_cart_interface *>(get_card_device());
}

READ8_MEMBER(nes_aladdin_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset, mem_mask);

	return 0xff;
}

// 128K for Dizzy The Adventurer, 256K for the others
bool nes_aladdin_slot_device::call_load()
{
	if (m_cart)
	{
		UINT8 *ROM = m_cart->get_cart_base();
		UINT32 size;

		if (!ROM)
			return IMAGE_INIT_FAIL;

		if (software_entry() == nullptr)
		{
			if (length() != 0x20010 && length() != 0x40010)
				return IMAGE_INIT_FAIL;

			UINT8 temp[0x40010];
			size = length() - 0x10;
			fread(&temp, length());
			memcpy(ROM, temp + 0x10, size);

			// double check that iNES files are really mapper 71 or 232
			{
				UINT8 mapper = (temp[6] & 0xf0) >> 4;
				mapper |= temp[7] & 0xf0;
				if (mapper != 71 && mapper != 232)
					return IMAGE_INIT_FAIL;
			}
		}
		else
		{
			if (get_software_region_length("rom") != 0x20000 && get_software_region_length("rom") != 0x40000)
				return IMAGE_INIT_FAIL;

			size = get_software_region_length("rom");
			memcpy(ROM, get_software_region("rom"), size);
		}

		m_cart->set_cart_size(size);
	}

	return IMAGE_INIT_PASS;
}


bool nes_aladdin_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry );
	return TRUE;
}

void nes_aladdin_slot_device::get_default_card_software(std::string &result)
{
	if (open_image_file(mconfig().options()))
	{
		const char *slot_string = "algn";
		UINT32 len = core_fsize(m_file);
		dynamic_buffer rom(len);
		UINT8 mapper;

		core_fread(m_file, &rom[0], len);

		mapper = (rom[6] & 0xf0) >> 4;
		mapper |= rom[7] & 0xf0;

//      if (mapper == 71)
//          slot_string = "algn";
		if (mapper == 232)
			slot_string = "algq";

		clear();

		result.assign(slot_string);
	}
	else
		software_get_default_slot(result, "algn");
}


//----------------------------------
//
//  Aladdin Minicart implementation
//
//----------------------------------

ROM_START( ade_rom )
	ROM_REGION(0x40000, "aderom", ROMREGION_ERASEFF)
ROM_END

const device_type NES_ALGN_ROM = &device_creator<nes_algn_rom_device>;
const device_type NES_ALGQ_ROM = &device_creator<nes_algq_rom_device>;

nes_algn_rom_device::nes_algn_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
						: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
							aladdin_cart_interface( mconfig, *this )
{
}

nes_algn_rom_device::nes_algn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: device_t(mconfig, NES_ALGN_ROM, "NES Aladdin Deck Enhancer ALGN ROM", tag, owner, clock, "nes_algn_rom", __FILE__),
							aladdin_cart_interface( mconfig, *this )
{
}

nes_algq_rom_device::nes_algq_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: nes_algn_rom_device(mconfig, NES_ALGQ_ROM, "NES Aladdin Deck Enhancer ALGQ ROM", tag, owner, clock, "nes_algq_rom", __FILE__), m_bank_base(0)
					{
}

void nes_algn_rom_device::device_start()
{
	m_rom = (UINT8*)memregion("aderom")->base();
	save_item(NAME(m_lobank));
}

void nes_algn_rom_device::device_reset()
{
	m_lobank = 0;
	m_hibank = 0x0f & m_rom_mask;
}

void nes_algq_rom_device::device_start()
{
	m_rom = (UINT8*)memregion("aderom")->base();
	save_item(NAME(m_lobank));
	save_item(NAME(m_hibank));
	save_item(NAME(m_bank_base));
}

void nes_algq_rom_device::device_reset()
{
	m_lobank = 0;
	m_hibank = 3;
	m_bank_base = 0;
}

const rom_entry *nes_algn_rom_device::device_rom_region() const
{
	return ROM_NAME( ade_rom );
}

UINT8 *nes_algn_rom_device::get_cart_base()
{
	return m_rom;
}

void nes_algn_rom_device::write_prg(UINT32 offset, UINT8 data)
{
	// m_hibank is fixed to the last available bank!
	if (offset >= 0x4000)
		m_lobank = data & m_rom_mask;
}

void nes_algq_rom_device::write_prg(UINT32 offset, UINT8 data)
{
	// here hibank & lobank variables are used differently
	// m_bank_base = 64K block
	// m_lobank = 16K page inside the block
	// m_hibank = 3rd page inside the block
	if (offset < 0x4000)
	{
		m_bank_base = ((data >> 3) & 3) << 2;
		m_lobank = m_bank_base | (m_lobank & 3);
		m_hibank = m_bank_base | 3;
	}
	else
		m_lobank = m_bank_base | (data & 3);
}


//-----------------------------------------------
//
//  Camerica/Codemasters Aladdin passthru
//  implementation
//
//-----------------------------------------------

const device_type NES_ALADDIN = &device_creator<nes_aladdin_device>;

nes_aladdin_device::nes_aladdin_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_ALADDIN, "NES Cart Camerica Aladdin PCB", tag, owner, clock, "nes_aladdin", __FILE__),
					m_subslot(*this, "ade_slot")
{
}


void nes_aladdin_device::device_start()
{
	common_start();
}

void nes_aladdin_device::pcb_reset()
{
	prg32(0xff);
	chr8(0, CHRRAM);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Camerica/Codemasters Aladdin Deck Enhancer

 iNES: mapper 71 & 232

 In MESS: Supported (but timing issues in some games)

 -------------------------------------------------*/

READ8_MEMBER(nes_aladdin_device::read_h)
{
	LOG_MMC(("aladdin read_h, offset: %04x\n", offset));
	// this shall be the proper code, but it's a bit slower, so we access directly the subcart below
	//return m_subslot->read(space, offset, mem_mask);

	if (m_subslot->m_cart)
		return m_subslot->m_cart->read(space, offset, mem_mask);
	else    // this is "fake" in the sense that we fill CPU space with 0xff if no Aladdin cart is loaded
		return hi_access_rom(offset);
}

WRITE8_MEMBER(nes_aladdin_device::write_h)
{
	LOG_MMC(("aladdin write_h, offset: %04x, data: %02x\n", offset, data));
	m_subslot->write_prg(offset, data);
}

//-------------------------------------------------
//  CART SLOT
//-------------------------------------------------

static SLOT_INTERFACE_START(ade_cart)
	SLOT_INTERFACE_INTERNAL("algn", NES_ALGN_ROM)
	SLOT_INTERFACE_INTERNAL("algq", NES_ALGQ_ROM)
SLOT_INTERFACE_END


MACHINE_CONFIG_FRAGMENT( camerica_aladdin )
	MCFG_ALADDIN_MINICART_ADD("ade_slot", ade_cart)
MACHINE_CONFIG_END

machine_config_constructor nes_aladdin_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( camerica_aladdin );
}
