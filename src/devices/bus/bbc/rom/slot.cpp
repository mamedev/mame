// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    BBC Micro ROM slot emulation

*********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ROMSLOT16, bbc_romslot16_device, "bbc_romslot16", "BBC Micro 16K ROM Slot")
DEFINE_DEVICE_TYPE(BBC_ROMSLOT32, bbc_romslot32_device, "bbc_romslot32", "BBC Micro 32K ROM Slot")


//**************************************************************************
//    DEVICE BBC_ROMSLOT CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_rom_interface - constructor
//-------------------------------------------------

device_bbc_rom_interface::device_bbc_rom_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "bbcrom")
	, m_rom(nullptr)
	, m_rom_size(0)
{
}


//-------------------------------------------------
//  ~device_bbc_rom_interface - destructor
//-------------------------------------------------

device_bbc_rom_interface::~device_bbc_rom_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the ROM
//-------------------------------------------------

void device_bbc_rom_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(BBC_ROM_REGION_TAG), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the RAM
//-------------------------------------------------

void device_bbc_rom_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
	device().save_item(NAME(m_ram));
}

//-------------------------------------------------
//  nvram_alloc - alloc the space for the NVRAM
//-------------------------------------------------

void device_bbc_rom_interface::nvram_alloc(uint32_t size)
{
	m_nvram.resize(size);
	device().save_item(NAME(m_nvram));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_romslot_device - constructor
//-------------------------------------------------
bbc_romslot_device::bbc_romslot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_bbc_rom_interface>(mconfig, *this)
	, m_cart(nullptr)
{
}

bbc_romslot16_device::bbc_romslot16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_romslot_device(mconfig, BBC_ROMSLOT16, tag, owner, clock)
{
}

bbc_romslot32_device::bbc_romslot32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_romslot_device(mconfig, BBC_ROMSLOT32, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_romslot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  call load
//-------------------------------------------------

std::pair<std::error_condition, std::string> bbc_romslot_device::call_load()
{
	if (m_cart)
	{
		uint32_t const size = !loaded_through_softlist() ? length() : get_software_region_length("rom");

		if (size % 0x2000)
			return std::make_pair(image_error::INVALIDLENGTH, "Invalid ROM size (must be a multiple of 8K)");

		uint8_t *base;
		const char *pcb_name = get_feature("slot");
		if (pcb_name && !strcmp(pcb_name, "ram"))
		{
			base = m_cart->get_ram_base();
		}
		else
		{
			m_cart->rom_alloc(size, tag());
			base = m_cart->get_rom_base();
		}

		if (!loaded_through_softlist())
			fread(base, size);
		else
			memcpy(base, get_software_region("rom"), size);

		m_cart->decrypt_rom();
	}

	return std::make_pair(std::error_condition(), std::string());
}

//-------------------------------------------------
//  call_unload
//-------------------------------------------------

void bbc_romslot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_base() && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}


//-------------------------------------------------
//  get default card software
//-------------------------------------------------

std::string bbc_romslot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}


//-------------------------------------------------
//  rom size
//-------------------------------------------------

uint32_t bbc_romslot_device::get_rom_size()
{
	if (m_cart)
		return m_cart->get_rom_size();
	else
		return 0;
}


//-------------------------------------------------
//  read - rom read
//-------------------------------------------------

uint8_t bbc_romslot_device::read(offs_t offset)
{
	if (m_cart)
		return m_cart->read(offset);
	else
		return 0xff;
}


//-------------------------------------------------
//  write - rom write
//-------------------------------------------------

void bbc_romslot_device::write(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->write(offset, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_rom )
//-------------------------------------------------

#include "rom.h"
#include "ram.h"
#include "nvram.h"
#include "datagem.h"
#include "dfs.h"
#include "genie.h"
//#include "gommc.h"
#include "pal.h"
//#include "ramagic.h"
#include "rtc.h"


//-------------------------------------------------
//  present - rom/ram selected
//-------------------------------------------------

bool bbc_romslot_device::present()
{
	if (m_cart && ((m_cart->device().type() == BBC_RAM) || m_cart->device().type() == BBC_NVRAM))
		return true;
	else
		return is_loaded() || loaded_through_softlist();
}


void bbc_rom_devices(device_slot_interface &device)
{
	device.option_add("ram", BBC_RAM);
	device.option_add_internal("rom", BBC_ROM);
	device.option_add_internal("nvram", BBC_NVRAM);
	device.option_add_internal("cciword", BBC_CCIWORD);
	device.option_add_internal("ccibase", BBC_CCIBASE);
	device.option_add_internal("ccispell", BBC_CCISPELL);
	device.option_add_internal("palqst", BBC_PALQST);
	device.option_add_internal("palwap", BBC_PALWAP);
	device.option_add_internal("palted", BBC_PALTED);
	device.option_add_internal("palabep", BBC_PALABEP);
	device.option_add_internal("palabe",  BBC_PALABE);
	device.option_add_internal("palmo2", BBC_PALMO2);
	device.option_add_internal("datagem", BBC_DATAGEM);
	device.option_add_internal("genie", BBC_PMSGENIE);
	//device.option_add_internal("gommc", BBC_GOMMC);
	device.option_add_internal("dfse00", BBC_DFSE00);
	//device.option_add_internal("ramagic", BBC_RAMAGIC);
	device.option_add_internal("stlrtc",  BBC_STLRTC);
	device.option_add_internal("pmsrtc", BBC_PMSRTC);
	device.option_add_internal("trilogy", BBC_TRILOGY);
}
