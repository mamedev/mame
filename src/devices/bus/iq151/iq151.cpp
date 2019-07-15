// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    IQ151 cartridge slot emulation

*********************************************************************/

#include "emu.h"
#include "iq151.h"

#define  LOG    0


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(IQ151CART_SLOT, iq151cart_slot_device, "iq151cart_slot", "IQ151 cartridge slot")

//**************************************************************************
//    IQ151 cartridge interface
//**************************************************************************

//-------------------------------------------------
//  device_iq151cart_interface - constructor
//-------------------------------------------------

device_iq151cart_interface::device_iq151cart_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_screen(nullptr)
{
}


//-------------------------------------------------
//  ~device_iq151cart_interface - destructor
//-------------------------------------------------

device_iq151cart_interface::~device_iq151cart_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151cart_slot_device - constructor
//-------------------------------------------------
iq151cart_slot_device::iq151cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IQ151CART_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_out_irq0_cb(*this)
	, m_out_irq1_cb(*this)
	, m_out_irq2_cb(*this)
	, m_out_irq3_cb(*this)
	, m_out_irq4_cb(*this)
	, m_out_drq_cb(*this)
	, m_cart(nullptr)
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  iq151cart_slot_device - destructor
//-------------------------------------------------

iq151cart_slot_device::~iq151cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151cart_slot_device::device_start()
{
	m_cart = dynamic_cast<device_iq151cart_interface *>(get_card_device());
	if (m_cart)
		m_cart->set_screen_device(m_screen);

	// resolve callbacks
	m_out_irq0_cb.resolve_safe();
	m_out_irq1_cb.resolve_safe();
	m_out_irq2_cb.resolve_safe();
	m_out_irq3_cb.resolve_safe();
	m_out_irq4_cb.resolve_safe();
	m_out_drq_cb.resolve_safe();

}


/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151cart_slot_device::read(offs_t offset, uint8_t &data)
{
	if (m_cart)
		m_cart->read(offset, data);
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

void iq151cart_slot_device::write(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->write(offset, data);
}

/*-------------------------------------------------
    IO read
-------------------------------------------------*/

void iq151cart_slot_device::io_read(offs_t offset, uint8_t &data)
{
	if (m_cart)
		m_cart->io_read(offset, data);
}


/*-------------------------------------------------
   IO write
-------------------------------------------------*/

void iq151cart_slot_device::io_write(offs_t offset, uint8_t data)
{
	if (m_cart)
		m_cart->io_write(offset, data);
}


/*-------------------------------------------------
   video update
-------------------------------------------------*/

void iq151cart_slot_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_cart)
		m_cart->video_update(bitmap, cliprect);
}


/*-------------------------------------------------
    call load
-------------------------------------------------*/

image_init_result iq151cart_slot_device::call_load()
{
	if (m_cart)
	{
		offs_t read_length;
		uint8_t *cart_base = m_cart->get_cart_base();

		if (cart_base != nullptr)
		{
			if (!loaded_through_softlist())
			{
				read_length = length();
				fread(m_cart->get_cart_base(), read_length);
			}
			else
			{
				read_length = get_software_region_length("rom");
				memcpy(m_cart->get_cart_base(), get_software_region("rom"), read_length);
			}
		}
		else
			return image_init_result::FAIL;
	}

	return image_init_result::PASS;
}

/*-------------------------------------------------
    get default card software
-------------------------------------------------*/

std::string iq151cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("basic6");
}
