// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MTX expansion emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MTX_EXP_SLOT, mtx_exp_slot_device, "mtx_exp_slot", "MTX expansion slot")


//**************************************************************************
//  DEVICE MTX_BUS PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_mtx_exp_interface - constructor
//-------------------------------------------------

device_mtx_exp_interface::device_mtx_exp_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "mtxexp")
{
	m_slot = dynamic_cast<mtx_exp_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mtx_exp_slot_device - constructor
//-------------------------------------------------

mtx_exp_slot_device::mtx_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MTX_EXP_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_mtx_exp_interface>(mconfig, *this)
	, m_program(*this, finder_base::DUMMY_TAG, -1)
	, m_io(*this, finder_base::DUMMY_TAG, -1)
	, m_busreq_handler(*this)
	, m_int_handler(*this)
	, m_nmi_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mtx_exp_slot_device::device_start()
{
	// resolve callbacks
	m_busreq_handler.resolve_safe();
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( mtx_exp_devices )
//-------------------------------------------------


// slot devices
//#include "fdx.h"
#include "sdx.h"


void mtx_expansion_devices(device_slot_interface &device)
{
	//device.option_add("fdx", MTX_FDX);         /* FDX Floppy Disc System */
	device.option_add("sdxbas", MTX_SDXBAS);   /* SDX Floppy Disc System (SDX ROM)*/
	device.option_add("sdxcpm", MTX_SDXCPM);   /* SDX Floppy Disc System (CP/M ROM and 80 column card) */
}
