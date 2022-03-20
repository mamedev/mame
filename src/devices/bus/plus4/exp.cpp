// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

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

DEFINE_DEVICE_TYPE(PLUS4_EXPANSION_SLOT, plus4_expansion_slot_device, "plus4_expansion_slot", "Plus/4 Expansion Port")



//**************************************************************************
//  DEVICE PLUS4_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_plus4_expansion_card_interface - constructor
//-------------------------------------------------

device_plus4_expansion_card_interface::device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "plus4exp"),
	m_c1l_size(0),
	m_c1h_size(0),
	m_c2l_size(0),
	m_c2h_size(0)
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

plus4_expansion_slot_device::plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLUS4_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_plus4_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_write_irq(*this),
	m_read_dma_cd(*this),
	m_write_dma_cd(*this),
	m_write_aec(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_read_dma_cd.resolve_safe(0xff);
	m_write_dma_cd.resolve_safe();
	m_write_aec.resolve_safe();
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

image_init_result plus4_expansion_slot_device::call_load()
{
	if (m_card)
	{
		if (!loaded_through_softlist())
		{
			// TODO
		}
		else
		{
			load_software_region("c1l", m_card->m_c1l);
			load_software_region("c1h", m_card->m_c1h);
			load_software_region("c2l", m_card->m_c2l);
			load_software_region("c2h", m_card->m_c2h);
			m_card->m_c1l_size = get_software_region_length("c1l");
			m_card->m_c1h_size = get_software_region_length("c1h");
			m_card->m_c2l_size = get_software_region_length("c2l");
			m_card->m_c2h_size = get_software_region_length("c2h");

			if ((m_card->m_c1l_size & (m_card->m_c1l_size - 1)) || (m_card->m_c1h_size & (m_card->m_c1h_size - 1)) || (m_card->m_c2l_size & (m_card->m_c2l_size - 1)) || (m_card->m_c2h_size & (m_card->m_c2h_size - 1)))
			{
				seterror(image_error::INVALIDIMAGE, "ROM size must be power of 2");
				return image_init_result::FAIL;
			}
		}
	}

	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string plus4_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

uint8_t plus4_expansion_slot_device::cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_card != nullptr)
	{
		data = m_card->plus4_cd_r(offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void plus4_expansion_slot_device::cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (m_card != nullptr)
	{
		m_card->plus4_cd_w(offset, data, ba, cs0, c1l, c1h, cs1, c2l, c2h);
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( plus4_expansion_cards )
//-------------------------------------------------

// slot devices
#include "c1551.h"
#include "sid.h"
#include "std.h"

void plus4_expansion_cards(device_slot_interface &device)
{
	device.option_add("c1551", C1551);
	device.option_add("sid", PLUS4_SID);

	// the following need ROMs from the software list
	device.option_add_internal("standard", PLUS4_STD);
}
