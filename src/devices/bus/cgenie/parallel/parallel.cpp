// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Parallel Slot

    20-pin slot

***************************************************************************/

#include "emu.h"
#include "parallel.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CG_PARALLEL_SLOT, cg_parallel_slot_device, "cg_parallel_slot", "Colour Genie Parallel Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  cg_parallel_slot_device - constructor
//-------------------------------------------------

cg_parallel_slot_device::cg_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CG_PARALLEL_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_cg_parallel_interface>(mconfig, *this),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  cg_parallel_slot_device - destructor
//-------------------------------------------------

cg_parallel_slot_device::~cg_parallel_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cg_parallel_slot_device::device_start()
{
	m_cart = get_card_device();
}


//**************************************************************************
//  I/O PORTS
//**************************************************************************

READ8_MEMBER( cg_parallel_slot_device::pa_r )
{
	if (m_cart)
		return m_cart->pa_r();
	else
		return 0xff;
}

WRITE8_MEMBER( cg_parallel_slot_device::pa_w )
{
	if (m_cart)
		m_cart->pa_w(data);
}

READ8_MEMBER( cg_parallel_slot_device::pb_r )
{
	if (m_cart)
		return m_cart->pb_r();
	else
		return 0xff;
}

WRITE8_MEMBER( cg_parallel_slot_device::pb_w )
{
	if (m_cart)
		m_cart->pb_w(data);
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cg_parallel_interface - constructor
//-------------------------------------------------

device_cg_parallel_interface::device_cg_parallel_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "cgeniepar")
{
	m_slot = dynamic_cast<cg_parallel_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_cg_parallel_interface - destructor
//-------------------------------------------------

device_cg_parallel_interface::~device_cg_parallel_interface()
{
}
