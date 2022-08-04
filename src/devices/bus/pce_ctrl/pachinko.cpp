// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    NEC PC Engine Coconuts Japan CJPC-101 Pachinko Controller

    This controller was originally released as a set with PC Engine CD
    game Pachio-kun Maboroshi no Densetsu. A near identical model was
    then put out for the Famicom.

    Functionally, this works like the non-turbo 2-button joypad with an
    additional spring loaded analog rotating trigger. The trigger is read
    after the standard buttons, much like a 2nd controller on a multitap.
    Hence, this controller cannot be used in conjunction with a multitap.

    TODO: according to games' packaging this controller works with all four
    Pachio-kun games. However, only Maboroshi no Densetsu and Juuban Shoubu
    prompt for controller type at boot. How to enable on the other two games?

**********************************************************************/

#include "emu.h"
#include "pachinko.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCE_PACHINKO, pce_pachinko_device, "pce_pachinko", "Coconuts Japan Pachinko Controller CJPC-101")


static INPUT_PORTS_START( pce_pachinko )
	// II is left of I on the original pad so we map them in reverse order
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p Button I")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p Button II")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("%p Select")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("%p Run")

	PORT_START("DPAD")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY

	PORT_START("TRIGGER")
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x80, 0xff) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_INVERT PORT_REVERSE
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pce_pachinko_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_pachinko );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_pachinko_device - constructor
//-------------------------------------------------

pce_pachinko_device::pce_pachinko_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_PACHINKO, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_buttons(*this, "BUTTONS"),
	m_dpad(*this, "DPAD"),
	m_trigger(*this, "TRIGGER"),
	m_counter(0),
	m_prev_sel(false),
	m_prev_clr(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_pachinko_device::device_start()
{
	save_item(NAME(m_counter));
	save_item(NAME(m_prev_sel));
	save_item(NAME(m_prev_clr));
}


//-------------------------------------------------
//  peripheral_r - joypad read
//-------------------------------------------------

u8 pce_pachinko_device::peripheral_r()
{
	u8 data = 0xf;

	if (!m_prev_clr)
	{
		switch (m_counter)
		{
			case 0:
				data = m_dpad->read();
				break;
			case 1:
				data = m_buttons->read();
				break;
			case 2:
				data = m_trigger->read() >> 4;
				break;
			case 3:
				data = m_trigger->read();
				break;
		}
	}

	return data;
}


//-------------------------------------------------
//  sel_w - MUXer select pin write
//-------------------------------------------------

void pce_pachinko_device::sel_w(int state)
{
	if (!m_prev_clr && m_prev_sel != state)
		m_counter = (m_counter + 1) & 7;

	m_prev_sel = state;
}


//-------------------------------------------------
//  clr_w - MUXer strobe pin write
//-------------------------------------------------

void pce_pachinko_device::clr_w(int state)
{
	if (m_prev_sel && !m_prev_clr && state)
		m_counter = 0;

	m_prev_clr = state;
}
