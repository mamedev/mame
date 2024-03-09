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
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"



//----------------------------------
//
//  Aladdin Cartslot implementation
//
//----------------------------------

//-------------------------------------------------
//  sub-cart interface
//-------------------------------------------------

aladdin_cart_interface::aladdin_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "aladdincart")
	, m_rom(nullptr)
	, m_rom_size(0)
	, m_lobank(0)
	, m_hibank(0)
	, m_rom_mask(0xff)
{
}

aladdin_cart_interface::~aladdin_cart_interface()
{
}

uint8_t aladdin_cart_interface::read(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[(m_lobank * 0x4000) + (offset & 0x3fff)];
	else
		return m_rom[(m_hibank * 0x4000) + (offset & 0x3fff)];
}

//-------------------------------------------------
//  sub-cart slot device
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_ALADDIN_SLOT, nes_aladdin_slot_device, "nes_ade_slot", "NES Aladdin Deck Enhancer Cartridge Slot")

nes_aladdin_slot_device::nes_aladdin_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NES_ALADDIN_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<aladdin_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
{
}

nes_aladdin_slot_device::~nes_aladdin_slot_device()
{
}


void nes_aladdin_slot_device::device_start()
{
	m_cart = get_card_device();
}

uint8_t nes_aladdin_slot_device::read(offs_t offset)
{
	if (m_cart)
		return m_cart->read(offset);

	return 0xff;
}

// 128K for Dizzy The Adventurer, 256K for the others
std::pair<std::error_condition, std::string> nes_aladdin_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *const ROM = m_cart->get_cart_base();
		if (!ROM)
			return std::make_pair(image_error::INTERNAL, std::string());

		uint32_t size;
		if (!loaded_through_softlist())
		{
			if (length() != 0x20010 && length() != 0x40010)
				return std::make_pair(image_error::INVALIDLENGTH, std::string());

			uint8_t temp[0x40010];
			size = length() - 0x10;
			fread(&temp, length());
			memcpy(ROM, temp + 0x10, size);

			// double check that iNES files are really mapper 71 or 232
			uint8_t mapper = (temp[6] & 0xf0) >> 4;
			mapper |= temp[7] & 0xf0;
			if (mapper != 71 && mapper != 232)
			{
				return std::make_pair(
						image_error::INVALIDIMAGE,
						util::string_format("Unsupported iNES mapper %u (must be 71 or 232)", mapper));
			}
		}
		else
		{
			size = get_software_region_length("rom");
			if (size != 0x2'0000 && size != 0x4'0000)
				return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 128K or 256K)");

			memcpy(ROM, get_software_region("rom"), size);
		}

		m_cart->set_cart_size(size);
	}

	return std::make_pair(std::error_condition(), std::string());
}


std::string nes_aladdin_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		uint64_t len;
		hook.image_file()->length(len); // FIXME: check error return, guard against excessively large files
		std::vector<uint8_t> rom(len);

		util::read(*hook.image_file(), &rom[0], len); // FIXME: check error return or read returning short

		uint8_t const mapper = ((rom[6] & 0xf0) >> 4) | (rom[7] & 0xf0);

		const char *slot_string = "algn";
//      if (mapper == 71)
//          slot_string = "algn";
		if (mapper == 232)
			slot_string = "algq";

		return std::string(slot_string);
	}

	return software_get_default_slot("algn");
}


//----------------------------------
//
//  Aladdin Minicart implementation
//
//----------------------------------

ROM_START( ade_rom )
	ROM_REGION(0x40000, "aderom", ROMREGION_ERASEFF)
ROM_END

DEFINE_DEVICE_TYPE(NES_ALGN_ROM, nes_algn_rom_device, "nes_algn_rom", "NES Aladdin Deck Enhancer ALGN ROM")
DEFINE_DEVICE_TYPE(NES_ALGQ_ROM, nes_algq_rom_device, "nes_algq_rom", "NES Aladdin Deck Enhancer ALGQ ROM")

nes_algn_rom_device::nes_algn_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, aladdin_cart_interface(mconfig, *this)
{
}

nes_algn_rom_device::nes_algn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_algn_rom_device(mconfig, NES_ALGN_ROM, tag, owner, clock)
{
}

nes_algq_rom_device::nes_algq_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_algn_rom_device(mconfig, NES_ALGQ_ROM, tag, owner, clock)
	, m_bank_base(0)
{
}

void nes_algn_rom_device::device_start()
{
	m_rom = (uint8_t*)memregion("aderom")->base();
	save_item(NAME(m_lobank));
}

void nes_algn_rom_device::device_reset()
{
	m_lobank = 0;
	m_hibank = 0x0f & m_rom_mask;
}

void nes_algq_rom_device::device_start()
{
	m_rom = (uint8_t*)memregion("aderom")->base();
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

const tiny_rom_entry *nes_algn_rom_device::device_rom_region() const
{
	return ROM_NAME( ade_rom );
}

uint8_t *nes_algn_rom_device::get_cart_base()
{
	return m_rom;
}

void nes_algn_rom_device::write_prg(uint32_t offset, uint8_t data)
{
	// m_hibank is fixed to the last available bank!
	if (offset >= 0x4000)
		m_lobank = data & m_rom_mask;
}

void nes_algq_rom_device::write_prg(uint32_t offset, uint8_t data)
{
	// here hibank & lobank variables are used differently
	// m_bank_base = 64K block
	// m_lobank = 16K page inside the block
	// m_hibank = 3rd page inside the block
	if (offset < 0x4000)
	{
		m_bank_base = (data & 0x18) >> 1;
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

DEFINE_DEVICE_TYPE(NES_ALADDIN, nes_aladdin_device, "nes_aladdin", "NES Cart Camerica Aladdin PCB")

nes_aladdin_device::nes_aladdin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_ALADDIN, tag, owner, clock)
	, m_subslot(*this, "ade_slot")
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

uint8_t nes_aladdin_device::read_h(offs_t offset)
{
	LOG("aladdin read_h, offset: %04x\n", offset);
	// this shall be the proper code, but it's a bit slower, so we access directly the subcart below
	//return m_subslot->read(offset);

	if (m_subslot->m_cart)
		return m_subslot->m_cart->read(offset);
	else    // this is "fake" in the sense that we fill CPU space with 0xff if no Aladdin cart is loaded
		return hi_access_rom(offset);
}

void nes_aladdin_device::write_h(offs_t offset, uint8_t data)
{
	LOG("aladdin write_h, offset: %04x, data: %02x\n", offset, data);
	m_subslot->write_prg(offset, data);
}

//-------------------------------------------------
//  CART SLOT
//-------------------------------------------------

static void ade_cart(device_slot_interface &device)
{
	device.option_add_internal("algn", NES_ALGN_ROM);
	device.option_add_internal("algq", NES_ALGQ_ROM);
}


void nes_aladdin_device::device_add_mconfig(machine_config &config)
{
	NES_ALADDIN_SLOT(config, m_subslot, ade_cart);
}
