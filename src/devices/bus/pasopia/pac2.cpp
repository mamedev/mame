// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Toshiba Pasopia PAC2 slot

    This is the smallest of the three Pasopia expansion connectors. Despite
    only providing access to a 4-by-8-bit space, this was mostly used for
    memory cartridges.

                                   ______
                       CDB 7   1  | .  . |  2   CDB 6
                         GND   3  | .  . |  4   CDB 5
                       CDB 4   5  | .  . |  6   CDB 3
                         GND   7  | .  . |  8   CDB 2
                       CDB 1   9  | .  . |  10  CDB 0
                      CDAD01  11  | .  . |  12  CDAD00
                         GND  13  | .  . |  14  CSELP2
                        CDRD  15  | .  . |  16  CDWR
                         GND  17  | .  . |  18  GND
                         +5V  19  | .  . |  20  +5V
                                   ------

****************************************************************************/

#include "emu.h"
#include "pac2.h"

#include "pac2exp.h"
#include "rampac2.h"
#include "rompac2.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PASOPIA_PAC2, pasopia_pac2_slot_device, "pasopia_pac2", "Pasopia PAC2 Slot")

//**************************************************************************
//  PAC2 SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  pasopia_pac2_slot_device - construction
//-------------------------------------------------

pasopia_pac2_slot_device::pasopia_pac2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PASOPIA_PAC2, tag, owner, clock)
	, device_single_card_slot_interface<pac2_card_interface>(mconfig, *this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void pasopia_pac2_slot_device::device_resolve_objects()
{
	m_card = get_card_device();
	if (m_card != nullptr)
		m_card->m_slot = this;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pasopia_pac2_slot_device::device_start()
{
}


//-------------------------------------------------
//  read - I/O read access
//-------------------------------------------------

u8 pasopia_pac2_slot_device::read(offs_t offset)
{
	if (m_card == nullptr)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Read from unconnected PAC2 (CDAD = %X)\n", machine().describe_context(), offset);
		return 0;
	}

	return m_card->pac2_read(offset);
}


//-------------------------------------------------
//  write - I/O write access
//-------------------------------------------------

void pasopia_pac2_slot_device::write(offs_t offset, u8 data)
{
	if (m_card != nullptr)
		m_card->pac2_write(offset, data);
}

//**************************************************************************
//  PAC2 CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  pac2_card_interface - construction
//-------------------------------------------------

pac2_card_interface::pac2_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pac2card")
	, m_slot(nullptr)
{
}


//-------------------------------------------------
//  interface_pre_start - called before the
//  device's own start function
//-------------------------------------------------

void pac2_card_interface::interface_pre_start()
{
	if (!m_slot)
		throw device_missing_dependencies();
}


//-------------------------------------------------
//  pac2_subslot_devices - add standard options
//  for subslots
//-------------------------------------------------

void pac2_subslot_devices(device_slot_interface &device)
{
	//device.option_add("ram4k", PASOPIA_PA7241);
	device.option_add("ram16k", PASOPIA_PA7243);
	device.option_add("ram32k", PASOPIA_PA7245);
	device.option_add("kanji", PASOPIA_PA7246);
	//device.option_add("kanji2", PASOPIA_PA7247);
	device.option_add("ram64k", PASOPIA_PA7248);
	//device.option_add("joy", PASOPIA_PA7390);
}


//-------------------------------------------------
//  pac2_default_devices - add standard options
//  for main slots
//-------------------------------------------------

void pac2_default_devices(device_slot_interface &device)
{
	device.option_add("exp", PASOPIA_PA7234);
	pac2_subslot_devices(device);
}
