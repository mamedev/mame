// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PLUS4_EXPANSION_SLOT = &device_creator<plus4_expansion_slot_device>;



//**************************************************************************
//  DEVICE PLUS4_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_plus4_expansion_card_interface - constructor
//-------------------------------------------------

device_plus4_expansion_card_interface::device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_c1l(*this, "c1l"),
		m_c1h(*this, "c1h"),
		m_c2l(*this, "c2l"),
		m_c2h(*this, "c2h"),
		m_c1l_mask(0),
		m_c1h_mask(0),
		m_c2l_mask(0),
		m_c2h_mask(0)
{
	m_slot = dynamic_cast<plus4_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_plus4_expansion_card_interface - destructor
//-------------------------------------------------

device_plus4_expansion_card_interface::~device_plus4_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_expansion_slot_device - constructor
//-------------------------------------------------

plus4_expansion_slot_device::plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PLUS4_EXPANSION_SLOT, "Expansion Port", tag, owner, clock, "plus4_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		device_image_interface(mconfig, *this),
		m_write_irq(*this),
		m_read_dma_cd(*this),
		m_write_dma_cd(*this),
		m_write_aec(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_plus4_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_read_dma_cd.resolve_safe(0xff);
	m_write_dma_cd.resolve_safe();
	m_write_aec.resolve_safe();

	// inherit bus clock
	if (clock() == 0)
	{
		plus4_expansion_slot_device *root = machine().device<plus4_expansion_slot_device>(PLUS4_EXPANSION_SLOT_TAG);
		assert(root);
		set_unscaled_clock(root->clock());
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_expansion_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

bool plus4_expansion_slot_device::call_load()
{
	if (m_card)
	{
		if (software_entry() == nullptr)
		{
			// TODO
		}
		else
		{
			load_software_region("c1l", m_card->m_c1l);
			load_software_region("c1h", m_card->m_c1h);
			load_software_region("c2l", m_card->m_c2l);
			load_software_region("c2h", m_card->m_c2h);
		}
	}

	return IMAGE_INIT_PASS;
}


//-------------------------------------------------
//  call_softlist_load -
//-------------------------------------------------

bool plus4_expansion_slot_device::call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry)
{
	load_software_part_region(*this, swlist, swname, start_entry);

	return true;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

void plus4_expansion_slot_device::get_default_card_software(std::string &result)
{
	software_get_default_slot(result, "standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

UINT8 plus4_expansion_slot_device::cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_card != nullptr)
	{
		data = m_card->plus4_cd_r(space, offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void plus4_expansion_slot_device::cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_card != nullptr)
	{
		m_card->plus4_cd_w(space, offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( plus4_expansion_cards )
//-------------------------------------------------

// slot devices
#include "c1551.h"
#include "sid.h"
#include "std.h"

SLOT_INTERFACE_START( plus4_expansion_cards )
	SLOT_INTERFACE("c1551", C1551)
	SLOT_INTERFACE("sid", PLUS4_SID)

	// the following need ROMs from the software list
	SLOT_INTERFACE_INTERNAL("standard", PLUS4_STD)
SLOT_INTERFACE_END
