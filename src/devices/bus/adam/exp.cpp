// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Expansion Port emulation

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_EXPANSION_SLOT = &device_creator<adam_expansion_slot_device>;



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_adam_expansion_slot_card_interface - constructor
//-------------------------------------------------

device_adam_expansion_slot_card_interface::device_adam_expansion_slot_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
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

adam_expansion_slot_device::adam_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ADAM_EXPANSION_SLOT, "ADAM expansion slot", tag, owner, clock, "adam_expansion_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_write_irq(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_adam_expansion_slot_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void adam_expansion_slot_device::device_reset()
{
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool adam_expansion_slot_device::call_load()
{
	if (m_card)
	{
		size_t size = 0;

		if (software_entry() == nullptr)
		{
			size = length();

			fread(m_card->m_rom, size);
		}
		else
		{
			load_software_region("rom", m_card->m_rom);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool adam_expansion_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

void adam_expansion_slot_device::get_default_card_software(std::string &result)
{
	software_get_default_slot(result, "standard");
}


//-------------------------------------------------
//  bd_r - buffered data read
//-------------------------------------------------

UINT8 adam_expansion_slot_device::bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (m_card != nullptr)
	{
		data = m_card->adam_bd_r(space, offset, data, bmreq, biorq, aux_rom_cs, cas1, cas2);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void adam_expansion_slot_device::bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (m_card != nullptr)
	{
		m_card->adam_bd_w(space, offset, data, bmreq, biorq, aux_rom_cs, cas1, cas2);
	}
}


// slot devices
#include "adamlink.h"
#include "ide.h"
#include "ram.h"

//-------------------------------------------------
//  SLOT_INTERFACE( adam_slot1_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( adam_slot1_devices )
	SLOT_INTERFACE("adamlink", ADAMLINK)
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( adam_slot2_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( adam_slot2_devices )
	SLOT_INTERFACE("ide", ADAM_IDE)
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( adam_slot3_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( adam_slot3_devices )
	SLOT_INTERFACE("ram", ADAM_RAM)
SLOT_INTERFACE_END
