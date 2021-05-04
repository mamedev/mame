// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 2 button joypad emulation

    Based on SMS controller port emulation (devices\bus\sms_ctrl\*.*)
	by Fabio Priuli,
	PC engine emulation (mame\*\pce.*)
	by Charles MacDonald, Wilbert Pol, Angelo Salese

    First party models:

    NEC PC Engine Pad (PI-PD001)
    No autofire, Bundled in PC Engine

    NEC Turbo Pad (PI-PD002 and Later models)
    Add autofire support

    NEC Turbo Stick (PI-PD4)
    Arcade joystick variation of Turbo Pad, with Seperated autofire
	buttons.
    Turbo Stick for TurboGrafx-16 has slow motion.

    Everything else PI-PD001 supports Autofire.

**********************************************************************/

#include "emu.h"
#include "joypad2.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCE_JOYPAD2, pce_joypad2_device, "pce_joypad2", "NEC PC Engine/TurboGrafx-16 2 Button Joypad")


static INPUT_PORTS_START( pce_joypad2 )
	// II is left of I on the original pad so we map them in reverse order
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Button I")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Button II")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("Select")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("Run")

	PORT_START("DIRECTION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pce_joypad2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_joypad2 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_joypad2_device - constructor
//-------------------------------------------------

pce_joypad2_device::pce_joypad2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_JOYPAD2, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_muxer(*this, "mux")
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void pce_joypad2_device::device_add_mconfig(machine_config &config)
{
	LS157(config, m_muxer); // TODO: actual part number
	m_muxer->a_in_callback().set_ioport("BUTTONS");
	m_muxer->b_in_callback().set_ioport("DIRECTION");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_joypad2_device::device_start()
{
}


//-------------------------------------------------
//  peripheral_r - joypad read
//-------------------------------------------------

u8 pce_joypad2_device::peripheral_r()
{
	return m_muxer->output_r();
}


//-------------------------------------------------
//  clk_w - MUXer select pin write
//-------------------------------------------------

void pce_joypad2_device::clk_w(int state)
{
	m_muxer->select_w(state);
}


//-------------------------------------------------
//  rst_w - MUXer strobe pin write
//-------------------------------------------------

void pce_joypad2_device::rst_w(int state)
{
	m_muxer->strobe_w(state);
}
