// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore VIC-1011A/B RS-232C Adapter emulation

**********************************************************************/

#include "emu.h"
#include "vic1011.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC1011, vic1011_device, "vic1011", "VIC-1011 RS-232C")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vic1011_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(vic1011_device::output_rxd));
	m_rs232->dcd_handler().set(FUNC(vic1011_device::output_h)).invert();
	m_rs232->cts_handler().set(FUNC(vic1011_device::output_k)).invert();
	m_rs232->dsr_handler().set(FUNC(vic1011_device::output_l)).invert();
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1011_device - constructor
//-------------------------------------------------

vic1011_device::vic1011_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VIC1011, tag, owner, clock)
	, device_pet_user_port_interface(mconfig, *this)
	, m_rs232(*this, "rs232")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1011_device::device_start()
{
}

WRITE_LINE_MEMBER( vic1011_device::output_rxd )
{
	output_b(state);
	output_c(state);
}

void vic1011_device::input_d(int state)
{
	m_rs232->write_rts(!state);
}

void vic1011_device::input_e(int state)
{
	m_rs232->write_dtr(!state);
}

void vic1011_device::input_j(int state)
{
	/// dcdout
}

void vic1011_device::input_m(int state)
{
	m_rs232->write_txd(state);
}
