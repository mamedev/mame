// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius Cartridge Port emulation

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_CARTRIDGE_SLOT, aquarius_cartridge_slot_device, "aquarius_cartridge_slot", "Aquarius Cartridge port")


//**************************************************************************
//  DEVICE AQUARIUS_CARTRIDGE CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_aquarius_cartridge_interface - constructor
//-------------------------------------------------

device_aquarius_cartridge_interface::device_aquarius_cartridge_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "aquariuscart")
	, m_rom(nullptr)
	, m_rom_size(0)
{
	m_slot = dynamic_cast<aquarius_cartridge_slot_device *>(device.owner());
}


//-------------------------------------------------
//  rom_alloc - alloc the space for the ROM
//-------------------------------------------------

void device_aquarius_cartridge_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(AQUARIUS_CART_ROM_REGION_TAG).c_str(), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_cartridge_slot_device - constructor
//-------------------------------------------------

aquarius_cartridge_slot_device::aquarius_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_CARTRIDGE_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_aquarius_cartridge_interface>(mconfig, *this)
	, m_cart(nullptr)
	, m_irq_handler(*this)
	, m_nmi_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_cartridge_slot_device::device_start()
{
	m_cart = get_card_device();

	// resolve callbacks
	m_irq_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}

//-------------------------------------------------
//  call_load
//-------------------------------------------------

image_init_result aquarius_cartridge_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t size = !loaded_through_softlist() ? length() : get_software_region_length("rom");

		if (size % 0x1000)
		{
			seterror(image_error::INVALIDIMAGE, "Invalid ROM size");
			return image_init_result::FAIL;
		}

		m_cart->rom_alloc(size, tag());

		if (!loaded_through_softlist())
			fread(m_cart->get_rom_base(), size);
		else
			memcpy(m_cart->get_rom_base(), get_software_region("rom"), size);
	}

	return image_init_result::PASS;
}

//-------------------------------------------------
//  get_default_card_software
//-------------------------------------------------

std::string aquarius_cartridge_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	static const unsigned char SC08_HEADER[16] = { 0x53, 0x43, 0x30, 0x38, 0x4b, 0x9c, 0xb5, 0xb0, 0xa8, 0x6c, 0xac, 0x64, 0xcc, 0xa8, 0x06, 0x70 };
	static const unsigned char SC16_HEADER[16] = { 0x53, 0x43, 0x31, 0x36, 0x4b, 0x9c, 0xb5, 0xb0, 0xa8, 0x6c, 0xac, 0x64, 0xcc, 0xa8, 0x08, 0x70 };

	if (hook.image_file())
	{
		uint64_t len;
		hook.image_file()->length(len); // FIXME: check error return

		const char *slot_string = "rom";
		if (len >= 0x10000)
		{
			uint8_t header[16];

			size_t actual;
			hook.image_file()->seek(len - 0x2000, SEEK_SET); // FIXME: check error return
			hook.image_file()->read(&header[0], 16, actual); // FIXME: check error return or read returning short

			// detect SuperCart header
			if (!memcmp(&header[0], SC08_HEADER, 16) || !memcmp(&header[0], SC16_HEADER, 16))
				slot_string = "sc1";
		}

		return std::string(slot_string);
	}
	else
		return software_get_default_slot("rom");
}

//-------------------------------------------------
//  iorq_r
//-------------------------------------------------

uint8_t aquarius_cartridge_slot_device::iorq_r(offs_t offset)
{
	if (m_cart)
		return m_cart->iorq_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  iorq_w
//-------------------------------------------------

void aquarius_cartridge_slot_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->iorq_w(offset, data);
}

//-------------------------------------------------
//  mreq_r
//-------------------------------------------------

uint8_t aquarius_cartridge_slot_device::mreq_r(offs_t offset)
{
	if (m_cart)
		return m_cart->mreq_r(offset);
	else
		return 0xff;
}

uint8_t aquarius_cartridge_slot_device::mreq_ce_r(offs_t offset)
{
	if (m_cart)
		return m_cart->mreq_ce_r(offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  mreq_w
//-------------------------------------------------

void aquarius_cartridge_slot_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->mreq_w(offset, data);
}

void aquarius_cartridge_slot_device::mreq_ce_w(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->mreq_ce_w(offset, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( aquarius_cartridge_devices )
//-------------------------------------------------


// slot devices
#include "c1541.h"
#include "mini.h"
#include "qdisk.h"
#include "ram.h"
#include "rom.h"
#include "supercart.h"


void aquarius_cartridge_devices(device_slot_interface &device)
{
	device.option_add("mini", AQUARIUS_MINI);
	device.option_add("ram4", AQUARIUS_RAM4);
	device.option_add("ram16", AQUARIUS_RAM16);
	device.option_add("ram16p", AQUARIUS_RAM16P);
	device.option_add("ram32", AQUARIUS_RAM32);
	device.option_add_internal("c1541", AQUARIUS_C1541);
	device.option_add_internal("qdisk", AQUARIUS_QDISK);
	device.option_add_internal("rom", AQUARIUS_ROM);
	device.option_add_internal("sc1", AQUARIUS_SC1);
}
