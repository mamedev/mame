// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Memotech ROMPAK

**********************************************************************/

#include "emu.h"
#include "rompak.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MTX_ROMPAK, mtx_rompak_device, "rompak", "MTX ROMPAK")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mtx_rompak_device - constructor
//-------------------------------------------------

mtx_rompak_device::mtx_rompak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MTX_ROMPAK, tag, owner, clock)
	, device_mtx_exp_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtx_rompak_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mtx_rompak_device::device_reset()
{
	machine().root_device().membank("rommap_bank1")->configure_entry(7, get_rom_base());
}
