// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**********************************************************************

    CCBUS Slot interface for PC-98xx family

**********************************************************************/

#include "emu.h"
#include "pc9801_cbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PC9801CBUS_SLOT, pc9801_slot_device, "pc9801_slot", "PC-9801 sound cbus slot")



//**************************************************************************
//  DEVICE PC9801 CARD INTERFACE
//**************************************************************************

#if 0
//-------------------------------------------------
//  device_pc9801cbus_card_interface - constructor
//-------------------------------------------------

device_pc9801cbus_card_interface::device_pc9801cbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_pc9801cbus_card_interface - destructor
//-------------------------------------------------

device_pc9801cbus_card_interface::~device_pc9801cbus_card_interface()
{
}
#endif


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc9801_slot_device - constructor
//-------------------------------------------------

pc9801_slot_device::pc9801_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PC9801CBUS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_cpu(*this, "^maincpu"),
	m_int_callback{{*this}, {*this}, {*this}, {*this}, {*this}, {*this}, {*this}}
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pc9801_slot_device::device_config_complete()
{
	// ...
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void pc9801_slot_device::device_resolve_objects()
{
	for (auto &cb : m_int_callback)
		cb.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc9801_slot_device::device_start()
{
//  m_card = dynamic_cast<device_pc9801_slot_card_interface *>(get_card_device());
}
