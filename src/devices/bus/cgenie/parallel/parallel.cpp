// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Parallel Slot

    20-pin slot

***************************************************************************/

#include "parallel.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PARALLEL_SLOT = &device_creator<parallel_slot_device>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  parallel_slot_device - constructor
//-------------------------------------------------

parallel_slot_device::parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PARALLEL_SLOT, "Parallel Slot", tag, owner, clock, "parallel_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  parallel_slot_device - destructor
//-------------------------------------------------

parallel_slot_device::~parallel_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void parallel_slot_device::device_start()
{
	m_cart = dynamic_cast<device_parallel_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void parallel_slot_device::device_reset()
{
}


//**************************************************************************
//  I/O PORTS
//**************************************************************************

READ8_MEMBER( parallel_slot_device::pa_r )
{
	if (m_cart)
		return m_cart->pa_r();
	else
		return 0xff;
}

WRITE8_MEMBER( parallel_slot_device::pa_w )
{
	if (m_cart)
		m_cart->pa_w(data);
}

READ8_MEMBER( parallel_slot_device::pb_r )
{
	if (m_cart)
		return m_cart->pb_r();
	else
		return 0xff;
}

WRITE8_MEMBER( parallel_slot_device::pb_w )
{
	if (m_cart)
		m_cart->pb_w(data);
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_parallel_interface - constructor
//-------------------------------------------------

device_parallel_interface::device_parallel_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<parallel_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_parallel_interface - destructor
//-------------------------------------------------

device_parallel_interface::~device_parallel_interface()
{
}
