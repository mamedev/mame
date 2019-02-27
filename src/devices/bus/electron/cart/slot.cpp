// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***********************************************************************************************************

        Electron Cartridge slot emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_CARTSLOT, electron_cartslot_device, "electron_cartslot", "Electron Cartridge Slot")


//**************************************************************************
//  DEVICE ELECTRON_CARTSLOT CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_electron_cart_interface - constructor
//-------------------------------------------------

device_electron_cart_interface::device_electron_cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_rom(nullptr),
		m_rom_size(0)
{
	m_slot = dynamic_cast<electron_cartslot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_electron_cart_interface - destructor
//-------------------------------------------------

device_electron_cart_interface::~device_electron_cart_interface()
{
}

//-------------------------------------------------
//  rom_alloc - alloc the space for the cart
//-------------------------------------------------

void device_electron_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		if (size <= 0x8000)
		{
			m_rom = device().machine().memory().region_alloc(std::string(tag).append(ELECTRON_CART_ROM_REGION_TAG).c_str(), 0x8000, 1, ENDIANNESS_LITTLE)->base();
			m_rom_size = 0x8000;
		}
		else
		{
			m_rom = device().machine().memory().region_alloc(std::string(tag).append(ELECTRON_CART_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
			m_rom_size = size;
		}
	}
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_electron_cart_interface::ram_alloc(uint32_t size)
{
	m_ram.resize(size);
}

//-------------------------------------------------
//  ram_alloc - alloc the space for the on-cart RAM
//-------------------------------------------------

void device_electron_cart_interface::nvram_alloc(uint32_t size)
{
	m_nvram.resize(size);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_cartslot_device - constructor
//-------------------------------------------------
electron_cartslot_device::electron_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELECTRON_CARTSLOT, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_slot_interface(mconfig, *this),
	m_cart(nullptr),
	m_irq_handler(*this),
	m_nmi_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_cartslot_device::device_start()
{
	m_cart = dynamic_cast<device_electron_cart_interface *>(get_card_device());

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_cartslot_device::device_reset()
{
}


//-------------------------------------------------
//  call load
//-------------------------------------------------

image_init_result electron_cartslot_device::call_load()
{
	if (m_cart)
	{
		if (!loaded_through_softlist())
		{
			uint32_t size = length();

			if (size % 0x2000)
			{
				seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
				return image_init_result::FAIL;
			}

			m_cart->rom_alloc(size, tag());
			fread(m_cart->get_rom_base(), size);
		}
		else
		{
			// standard 2x16K ROMs cartridges
			uint32_t upsize = get_software_region_length("uprom");
			uint32_t losize = get_software_region_length("lorom");

			// other RAM/ROM device cartridges
			uint32_t romsize = get_software_region_length("rom");
			uint32_t ramsize = get_software_region_length("ram");
			uint32_t nvramsize = get_software_region_length("nvram");

			if ((upsize % 0x2000 && upsize != 0) || (losize % 0x2000 && losize != 0) || (romsize % 0x2000 && romsize != 0))
			{
				seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
				return image_init_result::FAIL;
			}

			// load standard 2x16K ROM cartridges
			if (losize != 0 || upsize != 0)
			{
				m_cart->rom_alloc(0x8000, tag());
				if (losize)
					memcpy(m_cart->get_rom_base() + 0x0000, get_software_region("lorom"), losize);
				if (upsize)
					memcpy(m_cart->get_rom_base() + 0x4000, get_software_region("uprom"), upsize);
			}

			// load ROM region of device cartridge
			if (romsize != 0)
			{
				m_cart->rom_alloc(romsize, tag());
				memcpy(m_cart->get_rom_base(), get_software_region("rom"), romsize);
			}

			// load RAM region of device cartridge
			if (ramsize != 0)
			{
				m_cart->ram_alloc(ramsize);
				memcpy(m_cart->get_ram_base(), get_software_region("ram"), ramsize);
			}

			// load NVRAM region of device cartridge
			if (nvramsize != 0)
			{
				std::vector<uint8_t> default_nvram(nvramsize);
				memcpy(&default_nvram[0], get_software_region("nvram"), nvramsize);

				// load NVRAM (using default if no NVRAM exists)
				std::vector<uint8_t> temp_nvram(nvramsize);
				battery_load(&temp_nvram[0], nvramsize, &default_nvram[0]);

				// copy NVRAM into cartridge
				m_cart->nvram_alloc(nvramsize);
				memcpy(m_cart->get_nvram_base(), &temp_nvram[0], nvramsize);
			}
		}
	}

	return image_init_result::PASS;
}


//-------------------------------------------------
//  call_unload
//-------------------------------------------------

void electron_cartslot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_base() && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}


//-------------------------------------------------
//  get default card software
//-------------------------------------------------

std::string electron_cartslot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("std");
}


//-------------------------------------------------
//  read - cartridge read
//-------------------------------------------------

uint8_t electron_cartslot_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (m_cart != nullptr)
	{
		data = m_cart->read(offset, infc, infd, romqa, oe, oe2);
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge write
//-------------------------------------------------

void electron_cartslot_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (m_cart != nullptr)
	{
		m_cart->write(offset, data, infc, infd, romqa, oe, oe2);
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( electron_cart )
//-------------------------------------------------

#include "abr.h"
#include "ap34.h"
#include "ap5.h"
#include "aqr.h"
#include "click.h"
#include "cumana.h"
#include "mgc.h"
#include "peg400.h"
//#include "e2p.h"
#include "sndexp.h"
#include "sndexp3.h"
#include "sp64.h"
#include "stlefs.h"
#include "tube.h"
#include "std.h"


void electron_cart(device_slot_interface &device)
{
	device.option_add_internal("std", ELECTRON_STDCART);
	device.option_add_internal("abr", ELECTRON_ABR);
	device.option_add_internal("ap34", ELECTRON_AP34);
	device.option_add_internal("ap5", ELECTRON_AP5);
	device.option_add_internal("aqr", ELECTRON_AQR);
	device.option_add_internal("click", ELECTRON_CLICK);
	device.option_add_internal("cumana", ELECTRON_CUMANA);
	device.option_add_internal("mgc", ELECTRON_MGC);
	device.option_add_internal("peg400", ELECTRON_PEG400);
	//device.option_add_internal("e2p", ELECTRON_E2P);
	device.option_add_internal("sndexp", ELECTRON_SNDEXP);
	device.option_add_internal("sndexp3", ELECTRON_SNDEXP3);
	device.option_add_internal("sp64", ELECTRON_SP64);
	device.option_add_internal("stlefs", ELECTRON_STLEFS);
	device.option_add_internal("tube", ELECTRON_TUBE);
}
