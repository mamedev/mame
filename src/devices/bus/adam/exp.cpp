// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_EXPANSION_SLOT, adam_expansion_slot_device, "adam_expansion_slot", "ADAM expansion slot")



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_adam_expansion_slot_card_interface - constructor
//-------------------------------------------------

device_adam_expansion_slot_card_interface::device_adam_expansion_slot_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "adamexp"),
	m_rom(*this, "rom")
{
	m_slot = dynamic_cast<adam_expansion_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_expansion_slot_device - constructor
//-------------------------------------------------

adam_expansion_slot_device::adam_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADAM_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_adam_expansion_slot_card_interface>(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_write_irq(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_write_irq.resolve_safe();
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

image_init_result adam_expansion_slot_device::call_load()
{
	if (m_card)
	{
		size_t size;

		if (!loaded_through_softlist())
		{
			size = length();

			fread(m_card->m_rom, size);
		}
		else
		{
			load_software_region("rom", m_card->m_rom);
		}
	}

	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string adam_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  bd_r - buffered data read
//-------------------------------------------------

uint8_t adam_expansion_slot_device::bd_r(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (m_card != nullptr)
	{
		data = m_card->adam_bd_r(offset, data, bmreq, biorq, aux_rom_cs, cas1, cas2);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void adam_expansion_slot_device::bd_w(offs_t offset, uint8_t data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (m_card != nullptr)
	{
		m_card->adam_bd_w(offset, data, bmreq, biorq, aux_rom_cs, cas1, cas2);
	}
}


// slot devices
#include "adamlink.h"
#include "ide.h"
#include "ram.h"

//-------------------------------------------------
//  SLOT_INTERFACE( adam_slot1_devices )
//-------------------------------------------------

void adam_slot1_devices(device_slot_interface &device)
{
	device.option_add("adamlink", ADAMLINK);
}


//-------------------------------------------------
//  SLOT_INTERFACE( adam_slot2_devices )
//-------------------------------------------------

void adam_slot2_devices(device_slot_interface &device)
{
	device.option_add("ide", ADAM_IDE);
}


//-------------------------------------------------
//  SLOT_INTERFACE( adam_slot3_devices )
//-------------------------------------------------

void adam_slot3_devices(device_slot_interface &device)
{
	device.option_add("ram", ADAM_RAM);
}
