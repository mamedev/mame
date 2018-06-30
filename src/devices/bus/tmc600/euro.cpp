// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telercas Telmac TMC-600 euro bus emulation

**********************************************************************/

#include "emu.h"
#include "euro.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(TMC600_EURO_BUS_SLOT, tmc600_euro_bus_slot_t, "tmc600_euro_bus_slot", "TMC-600 euro bus slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_tmc600_euro_bus_card_interface - constructor
//-------------------------------------------------

device_tmc600_euro_bus_card_interface::device_tmc600_euro_bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<tmc600_euro_bus_slot_t *>(device.owner());
}


//-------------------------------------------------
//  tmc600_euro_bus_slot_t - constructor
//-------------------------------------------------

tmc600_euro_bus_slot_t::tmc600_euro_bus_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TMC600_EURO_BUS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmc600_euro_bus_slot_t::device_start()
{
	m_card = dynamic_cast<device_tmc600_euro_bus_card_interface *>(get_card_device());
}


//-------------------------------------------------
//  SLOT_INTERFACE( tmc600_euro_bus_cards )
//-------------------------------------------------

void tmc600_euro_bus_cards(device_slot_interface &device)
{
	//device.option_add("tmc710", TMC710); // 5-way expander
	//device.option_add("tmc720", TMC720); // 5-way expander (new model)
	//device.option_add("tmce200", TMCE200); // 8 KB RAM (CMOS)
	//device.option_add("tmce220", TMCE220); // 16/32 KB RAM/EPROM
	//device.option_add("tmce225", TMCE225); // 16 KB RAM (CMOS)
	//device.option_add("tmce250", TMCE250); // 32-way input/output
	//device.option_add("tmce260", TMCE260); // RS-232
	//device.option_add("tmce270", TMCE270); // generic ADC card (RCA CA3162 or CA3300, Teledyne 8700/01/02/03/04/05)
	//device.option_add("tmce2701", TMCE2701); // TMCE-270 with CA3162
	//device.option_add("tmce2702", TMCE2702); // TMCE-270 with CA3300
	//device.option_add("tmce280", TMCE280); // floppy disc controller for 3x 5.25" and 4x 8"
	//device.option_add("tmce285", TMCE285); // monochrome CRT controller (min. 80 cps)
	//device.option_add("tmce300", TMCE300); // slave computer with parallel I/O
	//device.option_add("tmce305", TMCE304); // slave computer with serial I/O
}
