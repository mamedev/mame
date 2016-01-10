// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL ROM cartridge port emulation

**********************************************************************/

#include "rom.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QL_ROM_CARTRIDGE_SLOT = &device_creator<ql_rom_cartridge_slot_t>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ql_rom_cartridge_card_interface - constructor
//-------------------------------------------------

device_ql_rom_cartridge_card_interface::device_ql_rom_cartridge_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_rom(*this, "rom"),
	m_romoeh(0)
{
	m_slot = dynamic_cast<ql_rom_cartridge_slot_t *>(device.owner());
}


//-------------------------------------------------
//  ~device_ql_rom_cartridge_card_interface - destructor
//-------------------------------------------------

device_ql_rom_cartridge_card_interface::~device_ql_rom_cartridge_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_rom_cartridge_slot_t - constructor
//-------------------------------------------------

ql_rom_cartridge_slot_t::ql_rom_cartridge_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QL_ROM_CARTRIDGE_SLOT, "QL ROM cartridge slot", tag, owner, clock, "ql_rom_cartridge_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	device_image_interface(mconfig, *this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_rom_cartridge_slot_t::device_start()
{
	m_card = dynamic_cast<device_ql_rom_cartridge_card_interface *>(get_card_device());
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool ql_rom_cartridge_slot_t::call_load()
{
	if (m_card)
	{
		size_t size;

		if (software_entry() == nullptr)
		{
			size = length();

			m_card->m_rom.allocate(size);
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

bool ql_rom_cartridge_slot_t::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string ql_rom_cartridge_slot_t::get_default_card_software()
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  SLOT_INTERFACE( ql_rom_cartridge_cards )
//-------------------------------------------------

// slot devices
#include "miracle_hd.h"
#include "std.h"

SLOT_INTERFACE_START( ql_rom_cartridge_cards )
	SLOT_INTERFACE("mhd", MIRACLE_HARD_DISK)

	// the following need ROMs from the software list
	SLOT_INTERFACE_INTERNAL("standard", QL_STANDARD_ROM_CARTRIDGE)
SLOT_INTERFACE_END
