// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Expansion Slot

    50-pin slot

***************************************************************************/

#include "emu.h"
#include "expansion.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CG_EXP_SLOT, cg_exp_slot_device, "cg_exp_slot", "Colour Genie Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  cg_exp - constructor
//-------------------------------------------------

cg_exp_slot_device::cg_exp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CG_EXP_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_cg_exp_interface>(mconfig, *this),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_io(*this, finder_base::DUMMY_TAG, -1),
	m_cart(nullptr),
	m_int_handler(*this),
	m_nmi_handler(*this),
	m_reset_handler(*this)
{
}

//-------------------------------------------------
//  cg_exp_slot_device - destructor
//-------------------------------------------------

cg_exp_slot_device::~cg_exp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cg_exp_slot_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_cg_exp_interface - constructor
//-------------------------------------------------

device_cg_exp_interface::device_cg_exp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "cgenieexp")
{
	m_slot = dynamic_cast<cg_exp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_cg_exp_interface - destructor
//-------------------------------------------------

device_cg_exp_interface::~device_cg_exp_interface()
{
}
