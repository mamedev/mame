// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL ROM cartridge port emulation

**********************************************************************/

#include "emu.h"
#include "rom.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(QL_ROM_CARTRIDGE_SLOT, ql_rom_cartridge_slot_device, "ql_rom_cartridge_slot", "QL ROM cartridge slot")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ql_rom_cartridge_card_interface - constructor
//-------------------------------------------------

device_ql_rom_cartridge_card_interface::device_ql_rom_cartridge_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "qlrom"),
	m_rom(*this, "rom"),
	m_slot(dynamic_cast<ql_rom_cartridge_slot_device *>(device.owner())),
	m_romoeh(0)
{
}


//-------------------------------------------------
//  ~device_ql_rom_cartridge_card_interface - destructor
//-------------------------------------------------

device_ql_rom_cartridge_card_interface::~device_ql_rom_cartridge_card_interface()
{
}


void device_ql_rom_cartridge_card_interface::interface_post_start()
{
	device().save_item(NAME(m_romoeh));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_rom_cartridge_slot_device - constructor
//-------------------------------------------------

ql_rom_cartridge_slot_device::ql_rom_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, QL_ROM_CARTRIDGE_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_ql_rom_cartridge_card_interface>(mconfig, *this),
	device_image_interface(mconfig, *this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void ql_rom_cartridge_slot_device::device_resolve_objects()
{
	m_card = get_card_device();
}

void ql_rom_cartridge_slot_device::device_start()
{
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

image_init_result ql_rom_cartridge_slot_device::call_load()
{
	if (m_card)
	{
		size_t size;

		if (!loaded_through_softlist())
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

	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string ql_rom_cartridge_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  SLOT_INTERFACE( ql_rom_cartridge_cards )
//-------------------------------------------------

// slot devices
#include "miracle_hd.h"
#include "std.h"

void ql_rom_cartridge_cards(device_slot_interface &device)
{
	device.option_add("mhd", MIRACLE_HARD_DISK);

	// the following need ROMs from the software list
	device.option_add_internal("standard", QL_STANDARD_ROM_CARTRIDGE);
}
