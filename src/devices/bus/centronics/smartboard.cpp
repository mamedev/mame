// license:BSD-3-Clause
// copyright-holders:hap
/*

Tasc SmartBoard SB30 (LPT interface)
The actual chessboard controller device is in devices/machine/smartboard.*

*/

#include "emu.h"
#include "smartboard.h"

#include "smartboard.lh"


DEFINE_DEVICE_TYPE(CENTRONICS_SMARTBOARD, centronics_smartboard_device, "centronics_smartboard", "Tasc SmartBoard SB30 Interface")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

centronics_smartboard_device::centronics_smartboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CENTRONICS_SMARTBOARD, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_smartboard(*this, "smartboard")
{ }


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_smartboard_device::device_add_mconfig(machine_config &config)
{
	TASC_SB30(config, m_smartboard).data_out().set(FUNC(centronics_smartboard_device::output_busy));
	config.set_default_layout(layout_smartboard);
}
