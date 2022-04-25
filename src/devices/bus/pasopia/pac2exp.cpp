// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Toshiba Pasopia PAC2 expander

    Not much is known about the internal circuitry of this 4-slot expansion.
    It was apparently designed primarily for the Pasopia 7, with the cord
    connecting the sidecar to the main unit through a hole in a modified
    slot cover.

    The one-based slot selection scheme is due to the need to avoid
    interfering with the 0x80 control byte needed to configure the PPI on
    the PA7241 RAM PAC2.

****************************************************************************/

#include "emu.h"
#include "pac2exp.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PASOPIA_PA7234, pasopia_pa7234_device, "pa7234", "PA7234 Pasopia PAC2 Expansion Unit")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  pasopia_pa7234_device - construction
//-------------------------------------------------

pasopia_pa7234_device::pasopia_pa7234_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PASOPIA_PA7234, tag, owner, clock)
	, pac2_card_interface(mconfig, *this)
	, m_slot(*this, "slot%u", 1U)
	, m_slot_selected(0)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void pasopia_pa7234_device::device_add_mconfig(machine_config &config)
{
	for (auto &slot : m_slot)
		PASOPIA_PAC2(config, slot, pac2_subslot_devices, nullptr);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pasopia_pa7234_device::device_start()
{
	save_item(NAME(m_slot_selected));
}

//**************************************************************************
//  PAC2 INTERFACE
//**************************************************************************

//-------------------------------------------------
//  pac2_read - I/O read access
//-------------------------------------------------

u8 pasopia_pa7234_device::pac2_read(offs_t offset)
{
	return m_slot[m_slot_selected]->read(offset);
}


//-------------------------------------------------
//  pac2_write - I/O write access
//-------------------------------------------------

void pasopia_pa7234_device::pac2_write(offs_t offset, u8 data)
{
	// Assume all writes are passed through
	m_slot[m_slot_selected]->write(offset, data);

	// 1, 2, 3, 4 or 7 may supposedly be written here
	if ((offset & 3) == 3 && (data & 0x07) != 0)
		m_slot_selected = (data - 1) & 0x03;
}
