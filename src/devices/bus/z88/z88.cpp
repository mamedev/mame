// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*********************************************************************

    z88.c

    Z88 cartridge slots emulation

*********************************************************************/

#include "emu.h"
#include "z88.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CLOSE_FLAP_TIME     attotime::from_msec(200)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(Z88CART_SLOT, z88cart_slot_device, "z88cart_slot", "Z88 Cartridge Slot")


//**************************************************************************
//    Z88 cartridges Interface
//**************************************************************************

//-------------------------------------------------
//  device_z88cart_interface - constructor
//-------------------------------------------------

device_z88cart_interface::device_z88cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "z88cart")
{
}


//-------------------------------------------------
//  ~device_z88cart_interface - destructor
//-------------------------------------------------

device_z88cart_interface::~device_z88cart_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z88cart_slot_device - constructor
//-------------------------------------------------
z88cart_slot_device::z88cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, Z88CART_SLOT, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_z88cart_interface>(mconfig, *this)
	, m_out_flp_cb(*this)
	, m_cart(nullptr)
	, m_flp_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z88cart_slot_device::device_start()
{
	m_cart = get_card_device();

	// resolve callbacks
	m_out_flp_cb.resolve_safe();

	m_flp_timer = timer_alloc(TIMER_FLP_CLEAR);
	m_flp_timer->reset();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void z88cart_slot_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_FLP_CLEAR)
	{
		// close the flap
		m_out_flp_cb(CLEAR_LINE);
	}
}

/*-------------------------------------------------
    call load
-------------------------------------------------*/

image_init_result z88cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint8_t *cart_base = m_cart->get_cart_base();

		if (cart_base != nullptr)
		{
			if (!loaded_through_softlist())
			{
				offs_t read_length = length();
				fread(cart_base + (m_cart->get_cart_size() - read_length), read_length);
			}
			else
			{
				offs_t read_length = get_software_region_length("rom");
				memcpy(cart_base + (m_cart->get_cart_size() - read_length), get_software_region("rom"), read_length);
			}
		}
		else
		{
			return image_init_result::FAIL;
		}
	}

	// open the flap
	m_out_flp_cb(ASSERT_LINE);

	// setup the timer for close the flap
	m_flp_timer->adjust(CLOSE_FLAP_TIME);

	return image_init_result::PASS;
}


/*-------------------------------------------------
    call_unload
-------------------------------------------------*/

void z88cart_slot_device::call_unload()
{
	if (m_cart)
	{
		size_t cart_size = m_cart->get_cart_size();
		if (cart_size > 0)
		{
			memset(m_cart->get_cart_base(), 0xff, cart_size);
		}
	}

	// open the flap
	m_out_flp_cb(ASSERT_LINE);

	// setup the timer for close the flap
	m_flp_timer->adjust(CLOSE_FLAP_TIME);
}


/*-------------------------------------------------
    get default card software
-------------------------------------------------*/

std::string z88cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("128krom");
}


/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(z88cart_slot_device::read)
{
	if (m_cart)
		return m_cart->read(space, offset);
	else
		return 0xff;
}


/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(z88cart_slot_device::write)
{
	if (m_cart)
		m_cart->write(space, offset, data);
}


/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* z88cart_slot_device::get_cart_base()
{
	if (m_cart)
		return m_cart->get_cart_base();
	else
		return nullptr;
}
