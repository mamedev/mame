// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***********************************************************************************************************

        BBC Master Cartridge slot emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "slot.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BBCM_CARTSLOT, bbc_cartslot_device, "bbc_cartslot", "BBC Master Cartridge Slot")


//**************************************************************************
//  DEVICE ELECTRON_CARTSLOT CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_cart_interface - constructor
//-------------------------------------------------

device_bbc_cart_interface::device_bbc_cart_interface(const machine_config &mconfig, device_t &device)
	: device_electron_cart_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_cartslot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_cart_interface - destructor
//-------------------------------------------------

device_bbc_cart_interface::~device_bbc_cart_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cartslot_device - constructor
//-------------------------------------------------
bbc_cartslot_device::bbc_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	electron_cartslot_device(mconfig, BBCM_CARTSLOT, tag, owner, clock)
{
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbcm_cart )
//-------------------------------------------------

#include "bus/electron/cart/std.h"
#include "bus/electron/cart/abr.h"
#include "bus/electron/cart/aqr.h"
#include "click.h"
#include "mastersd.h"
#include "mega256.h"
#include "mr8000.h"
#include "msc.h"


void bbcm_cart(device_slot_interface &device)
{
	device.option_add_internal("std", ELECTRON_STDCART);
	device.option_add_internal("abr", ELECTRON_ABR);
	device.option_add_internal("aqr", ELECTRON_AQR);
	device.option_add_internal("click", BBC_CLICK);
	device.option_add_internal("mastersd", BBC_MASTERSD);
	device.option_add_internal("mega256", BBC_MEGA256);
	device.option_add_internal("mr8000", BBC_MR8000);
	device.option_add_internal("msc", BBC_MSC);
}
