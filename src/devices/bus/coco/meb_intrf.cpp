// license:BSD-3-Clause
// copyright-holders:tim lindner
/*********************************************************************

    meb_intrf.h

    CRC / Disto Mini Expansion Bus management

    Mini Expansion Bus
        17 pin in-line header
            pin
            1   reset
            2   e clock
            3   address 0
            4   address 1
            5   data 0
            6   data 1
            7   data 2
            8   data 3
            9   data 4
            10  data 5
            11  data 6
            12  data 7
            13  chip enable*
            14  ground
            15  r/w*
            16  +5v
            17  address 2

        There is also one additional pin for the CART signal

        Addresses active: $FF50 to $FF57

    This was implemented in the Super (floppy) Controller I and II,
    and in the ram disk expansion pak.

*********************************************************************/

#include "emu.h"
#include "meb_intrf.h"

#include "meb_rtime.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DISTOMEB_SLOT, distomeb_slot_device, "distomeb_slot", "Disto Mini Expansion Bus")


//-------------------------------------------------
//  distomeb_slot_device - constructor
//-------------------------------------------------
distomeb_slot_device::distomeb_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DISTOMEB_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_distomeb_interface>(mconfig, *this)
	, m_cart_callback(*this)
	, m_cart(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void distomeb_slot_device::device_start()
{
	m_cart = get_card_device();
	m_cart_callback.resolve_safe();

	save_item(NAME(m_cart_line));
}


//-------------------------------------------------
//  meb_read
//-------------------------------------------------

u8 distomeb_slot_device::meb_read(offs_t offset)
{
	u8 result = 0x00;
	if (m_cart)
		result = m_cart->meb_read(offset);
	return result;
}


//-------------------------------------------------
//  meb_write
//-------------------------------------------------


void distomeb_slot_device::meb_write(offs_t offset, u8 data)
{
	if (m_cart)
		m_cart->meb_write(offset, data);
}

//-------------------------------------------------
//  set_cart_line
//-------------------------------------------------

WRITE_LINE_MEMBER(distomeb_slot_device::set_cart_line)
{
	m_cart_line = state;
	m_cart_callback(state);
}


//**************************************************************************
//  DEVICE DISTO MEB INTERFACE - Implemented by devices that plug into
//  Disto Mini Expansion Bus
//**************************************************************************

template class device_finder<device_distomeb_interface, false>;
template class device_finder<device_distomeb_interface, true>;


//-------------------------------------------------
//  device_distomeb_interface - constructor
//-------------------------------------------------

device_distomeb_interface::device_distomeb_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "distomeb")
	, m_owning_slot(dynamic_cast<distomeb_slot_device *>(device.owner()))
{
}


//-------------------------------------------------
//  ~device_distomeb_interface - destructor
//-------------------------------------------------

device_distomeb_interface::~device_distomeb_interface()
{
}


//-------------------------------------------------
//  interface_pre_start
//-------------------------------------------------

void device_distomeb_interface::interface_pre_start()
{
	if (!m_owning_slot)
		throw emu_fatalerror("Expected device().owner() to be of type distomeb_slot_device");
}


//-------------------------------------------------
//  meb_read - Addresses active: $FF50 to $FF57
//-------------------------------------------------

u8 device_distomeb_interface::meb_read(offs_t offset)
{
	return 0x00;
}


//-------------------------------------------------
//  cts_write - Addresses active: $FF50 to $FF57
//-------------------------------------------------

void device_distomeb_interface::meb_write(offs_t offset, u8 data)
{
}


//-------------------------------------------------
//  set_cart_value
//-------------------------------------------------

void device_distomeb_interface::set_cart_value(int value)
{
	m_owning_slot->set_cart_line(value);
}


//-------------------------------------------------
//  disto_meb_add_basic_devices
//-------------------------------------------------

void disto_meb_add_basic_devices(device_slot_interface &device)
{
	// basic devices
	device.option_add("rtime", DISTOMEB_RTIME);
}
