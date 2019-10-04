// license:BSD-3-Clause
// copyright-holders:hap
/*

The ChessMachine EC by Tasc
External module with ARM2 CPU, also sold under the Mephisto brand by H+G

see chessmachine_device for technical notes

*/

#include "emu.h"
#include "chessmec.h"


DEFINE_DEVICE_TYPE(CENTRONICS_CHESSMEC, centronics_chessmec_device, "centronics_chessmec", "The ChessMachine EC")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

centronics_chessmec_device::centronics_chessmec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CENTRONICS_CHESSMEC, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_chessm(*this, "chessm")
{ }



//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_chessmec_device::device_add_mconfig(machine_config &config)
{
	CHESSMACHINE(config, m_chessm, 15'000'000);
	m_chessm->data_out().set(FUNC(centronics_chessmec_device::output_busy));
}
