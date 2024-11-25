// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision cartridge port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECOVISION_CARTRIDGE_SLOT, colecovision_cartridge_slot_device, "coleco_cartridge_port", "ColecoVision cartridge port")



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_colecovision_cartridge_interface - constructor
//-------------------------------------------------

device_colecovision_cartridge_interface::device_colecovision_cartridge_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "colecocart"),
	m_rom(nullptr),
	m_rom_size(0)
{
	m_slot = dynamic_cast<colecovision_cartridge_slot_device *>(device.owner());
}

void device_colecovision_cartridge_interface::rom_alloc(size_t size)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(":coleco_cart:rom", size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  colecovision_cartridge_slot_device - constructor
//-------------------------------------------------

colecovision_cartridge_slot_device::colecovision_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECOVISION_CARTRIDGE_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_colecovision_cartridge_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void colecovision_cartridge_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> colecovision_cartridge_slot_device::call_load()
{
	if (m_card)
	{
		size_t const size = !loaded_through_softlist() ? length() : get_software_region_length("rom");
		m_card->rom_alloc(size);

		if (!loaded_through_softlist())
		{
			fread(m_card->m_rom, size);
		}
		else
		{
			// TODO 8000/a000/c000/e000
			memcpy(m_card->m_rom, get_software_region("rom"), size);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string colecovision_cartridge_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		uint64_t length;
		hook.image_file()->length(length); // FIXME: check error return

		if (length == 0x100000 || length == 0x200000)
			return software_get_default_slot("xin1");

		if (length > 0x8000) // Assume roms longer than 32K are megacarts.
			return software_get_default_slot("megacart");
	}

	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t colecovision_cartridge_slot_device::read(offs_t offset, int _8000, int _a000, int _c000, int _e000)
{
	uint8_t data = 0xff;

	if (m_card)
		data = m_card->read(offset, _8000, _a000, _c000, _e000);

	return data;
}


//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void colecovision_cartridge_slot_device::write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000)
{
	if (m_card)
		m_card->write(offset, data, _8000, _a000, _c000, _e000);
}


//-------------------------------------------------
//  SLOT_INTERFACE( colecovision_cartridges )
//-------------------------------------------------

#include "activision.h"
#include "megacart.h"
#include "std.h"
#include "xin1.h"

void colecovision_cartridges(device_slot_interface &device)
{
	// the following need ROMs from the software list
	device.option_add_internal("activision", COLECOVISION_ACTIVISION);
	device.option_add_internal("activision_256b", COLECOVISION_ACTIVISION_256B);
	device.option_add_internal("activision_32k", COLECOVISION_ACTIVISION_32K);
	device.option_add_internal("megacart", COLECOVISION_MEGACART);
	device.option_add_internal("standard", COLECOVISION_STANDARD);
	device.option_add_internal("xin1", COLECOVISION_XIN1);
}
