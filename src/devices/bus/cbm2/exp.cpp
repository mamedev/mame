// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Expansion Port emulation

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CBM2_EXPANSION_SLOT = &device_creator<cbm2_expansion_slot_device>;



//**************************************************************************
//  DEVICE CBM2_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cbm2_expansion_card_interface - constructor
//-------------------------------------------------

device_cbm2_expansion_card_interface::device_cbm2_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3")
{
	m_slot = dynamic_cast<cbm2_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_cbm2_expansion_card_interface - destructor
//-------------------------------------------------

device_cbm2_expansion_card_interface::~device_cbm2_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cbm2_expansion_slot_device - constructor
//-------------------------------------------------

cbm2_expansion_slot_device::cbm2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, CBM2_EXPANSION_SLOT, "CBM-II expansion port", tag, owner, clock, "cbm2_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cbm2_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_cbm2_expansion_card_interface *>(get_card_device());

	// inherit bus clock
	if (clock() == 0)
	{
		cbm2_expansion_slot_device *root = machine().device<cbm2_expansion_slot_device>(CBM2_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cbm2_expansion_slot_device::device_reset()
{
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool cbm2_expansion_slot_device::call_load()
{
	size_t size = 0;

	if (m_card)
	{
		if (software_entry() == nullptr)
		{
			size = length();

			if (!core_stricmp(filetype(), "20"))
			{
				m_card->m_bank1.allocate(size);
				fread(m_card->m_bank1, size);
			}
			else if (!core_stricmp(filetype(), "40"))
			{
				m_card->m_bank2.allocate(size);
				fread(m_card->m_bank2, size);
			}
			else if (!core_stricmp(filetype(), "60"))
			{
				m_card->m_bank3.allocate(size);
				fread(m_card->m_bank3, size);
			}
		}
		else
		{
			load_software_region("bank1", m_card->m_bank1);
			load_software_region("bank2", m_card->m_bank2);
			load_software_region("bank3", m_card->m_bank3);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool cbm2_expansion_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

void cbm2_expansion_slot_device::get_default_card_software(std::string &result)
{
	software_get_default_slot(result, "standard");
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

UINT8 cbm2_expansion_slot_device::read(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (m_card != nullptr)
	{
		data = m_card->cbm2_bd_r(space, offset, data, csbank1, csbank2, csbank3);
	}

	return data;
}


//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void cbm2_expansion_slot_device::write(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3)
{
	if (m_card != nullptr)
	{
		m_card->cbm2_bd_w(space, offset, data, csbank1, csbank2, csbank3);
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( cbm2_expansion_cards )
//-------------------------------------------------

// slot devices
#include "24k.h"
#include "hrg.h"
#include "std.h"

SLOT_INTERFACE_START( cbm2_expansion_cards )
	SLOT_INTERFACE("24k", CBM2_24K)
	SLOT_INTERFACE("hrga", CBM2_HRG_A)
	SLOT_INTERFACE("hrgb", CBM2_HRG_B)
	SLOT_INTERFACE_INTERNAL("standard", CBM2_STD)
SLOT_INTERFACE_END
