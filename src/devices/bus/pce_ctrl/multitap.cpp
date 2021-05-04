// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 Multi Tap emulation

    Based on SMS controller port emulation (devices\bus\sms_ctrl\*.*)
	by Fabio Priuli,
	PC engine emulation (mame\*\pce.*)
	by Charles MacDonald, Wilbert Pol, Angelo Salese

    First party model (PI-PD003, and US released TurboTap and DuoTap)
    has allowed up to 5 controllers, Third-party Multi Taps are has
    allowed up to 2-4 controllers, and also compatible?

**********************************************************************/

#include "emu.h"
#include "multitap.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCE_MULTITAP, pce_multitap_device, "pce_multitap", "NEC PC Engine/TurboGrafx-16 Multi Tap")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_multitap_device - constructor
//-------------------------------------------------

pce_multitap_device::pce_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_MULTITAP, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_subctrl_port(*this, "ctrl%u", 1U),
	m_read_state(0),
	m_prev_sel(0)
{
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pce_multitap_device::device_add_mconfig(machine_config &config)
{
	for (auto & elem : m_subctrl_port)
		PCE_CONTROL_PORT(config, elem, pce_control_port_devices, "joypad2");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_multitap_device::device_start()
{
	save_item(NAME(m_read_state));
	save_item(NAME(m_prev_sel));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pce_multitap_device::device_reset()
{
	m_read_state = 0;
	m_prev_sel = false;
}


//-------------------------------------------------
//  peripheral_r - multitap read
//-------------------------------------------------

u8 pce_multitap_device::peripheral_r()
{
	u8 data = 0xf;
	if (m_read_state < 5) // up to 5 controller ports
		data = m_subctrl_port[m_read_state]->port_r();

	return data;
}


//-------------------------------------------------
//  clk_w - Clock pin write, with port select
//-------------------------------------------------

void pce_multitap_device::clk_w(int state)
{
	for (auto & elem : m_subctrl_port)
		elem->clk_w(state);

	// bump counter on a low-to-high transition of Clock bit
	if ((!m_prev_sel) && state)
		m_read_state = (m_read_state + 1) & 7;

	m_prev_sel = state;
}


//-------------------------------------------------
//  rst_w - Reset pin write, with reset multitap
//-------------------------------------------------

void pce_multitap_device::rst_w(int state)
{
	for (auto & elem : m_subctrl_port)
		elem->rst_w(state);

	// clear counter if Reset bit is set
	if (state)
		m_read_state = 0;
}
