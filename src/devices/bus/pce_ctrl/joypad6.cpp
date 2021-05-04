// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 6 button joypad emulation

    Based on SMS controller port emulation (devices\bus\sms_ctrl\*.*)
	by Fabio Priuli,
	PC engine emulation (mame\*\pce.*)
	by Charles MacDonald, Wilbert Pol, Angelo Salese

    There's 2 first party 6 button joypad models:

    NEC Avenue Pad 6 (NAPD-1002)
	- Supports autofire for Button I, II, Run (slow motion)

    NEC Arcade Pad 6 (PCE-TP1)
    - Supports autofire for Button I-VI, Bundled in PC Engine Duo-RX

**********************************************************************/

#include "emu.h"
#include "joypad6.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCE_JOYPAD6, pce_joypad6_device, "pce_joypad6", "NEC PC Engine/TurboGrafx-16 6 Button Joypad")


static INPUT_PORTS_START( pce_joypad6 )
	// II is left of I on the original pad so we map them in reverse order
	PORT_START("BUTTONS_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button I")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button II")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("Select")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("Run")

	PORT_START("DIRECTION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY

	PORT_START("BUTTONS_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Button III")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Button IV")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Button V")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Button VI")

	PORT_START("JOY_MODE")
	PORT_CONFNAME( 0x01, 0x00, "Joypad Mode" ) PORT_CHANGED_MEMBER(DEVICE_SELF, pce_joypad6_device, joypad_mode_changed, 0)
	PORT_CONFSETTING( 0x00, "2-buttons mode" )
	PORT_CONFSETTING( 0x01, "6-buttons mode" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pce_joypad6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_joypad6 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_joypad6_device - constructor
//-------------------------------------------------

pce_joypad6_device::pce_joypad6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_JOYPAD6, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_muxer(*this, "mux_%u", 0U),
	m_joypad_mode(*this, "JOY_MODE")
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void pce_joypad6_device::device_add_mconfig(machine_config &config)
{
	LS157(config, m_muxer[0]); // TODO: actual part number
	m_muxer[0]->a_in_callback().set_ioport("BUTTONS_0");
	m_muxer[0]->b_in_callback().set_ioport("DIRECTION");

	LS157(config, m_muxer[1]); // TODO: actual part number
	m_muxer[1]->a_in_callback().set_ioport("BUTTONS_1");
	m_muxer[1]->b_in_callback().set_constant(0x0); //6-button pad header

	LS157(config, m_muxer[2]); // TODO: actual part number
	m_muxer[2]->a_in_callback().set(m_muxer[0], FUNC(ls157_device::output_r));
	m_muxer[2]->b_in_callback().set(m_muxer[1], FUNC(ls157_device::output_r));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_joypad6_device::device_start()
{
	save_item(NAME(m_button_sel));
	save_item(NAME(m_prev_clr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pce_joypad6_device::device_reset()
{
	m_button_sel = 0;
	m_prev_clr = false;
}


//-------------------------------------------------
//  peripheral_r - joypad read
//-------------------------------------------------

u8 pce_joypad6_device::peripheral_r()
{
	return m_muxer[2]->output_r();
}


//-------------------------------------------------
//  clk_w - MUXer select pin write
//-------------------------------------------------

void pce_joypad6_device::clk_w(int state)
{
	m_muxer[0]->select_w(state);
	m_muxer[1]->select_w(state);
}


//-------------------------------------------------
//  rst_w - MUXer strobe pin write, toggle button
//  set and autofire control
//-------------------------------------------------

void pce_joypad6_device::rst_w(int state)
{
	m_muxer[0]->strobe_w(state);
	m_muxer[1]->strobe_w(state);
	if ((!m_prev_clr) && state)
		m_button_sel = (m_button_sel + 1) & 0x7; // Toggle buttons/autofire, connected to 74xx163 (TODO: Actual part number); QD pin not used

	m_prev_clr = state;
	buttonset_update();
}


//-------------------------------------------------
//  buttonset_update - toggle buttonsets
//-------------------------------------------------

void pce_joypad6_device::buttonset_update()
{
	m_muxer[2]->select_w((m_joypad_mode->read() & 1) ? BIT(m_button_sel, 0) : 0);
}


//-------------------------------------------------
//  joypad_mode_changed
//-------------------------------------------------

INPUT_CHANGED_MEMBER(pce_joypad6_device::joypad_mode_changed)
{
	buttonset_update();
}
