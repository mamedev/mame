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
    Add autofire support, Bundled in PC Engine CoreGrafx and later
    models

    NEC Turbo Stick (PI-PD4)
    Arcade joystick variation of Turbo Pad, with separated autofire
    buttons.
    Turbo Stick for TurboGrafx-16 has slow motion.

    Everything else PI-PD001 supports Autofire.

    TODO:
    - Needs verifications for Part numbers

**********************************************************************/

#include "emu.h"
#include "joypad2.h"

#include "machine/74157.h"


namespace {

INPUT_PORTS_START( pce_joypad2 )
	// II is left of I on the original pad so we map them in reverse order
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p Button I")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p Button II")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("%p Run")

	PORT_START("DIRECTION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
INPUT_PORTS_END


INPUT_PORTS_START( pce_joypad2_turbo )
	PORT_INCLUDE( pce_joypad2 )

	PORT_START("TURBO")
	PORT_CONFNAME( 0x03, 0x00, "Button I Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, "Slow" )
	PORT_CONFSETTING(    0x03, "Fast" )
	PORT_CONFNAME( 0x0c, 0x00, "Button II Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, "Slow" )
	PORT_CONFSETTING(    0x0c, "Fast" )
INPUT_PORTS_END



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_joypad2_device

class pce_joypad2_device : public device_t, public device_pce_control_port_interface
{
public:
	// construction/destruction
	pce_joypad2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// construction/destruction
	pce_joypad2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

	// devices
	required_device<ls157_device> m_muxer;
};


// ======================> pce_joypad2_turbo_device

class pce_joypad2_turbo_device : public pce_joypad2_device
{
public:
	// construction/destruction
	pce_joypad2_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_pce_control_port_interface overrides
	virtual void clr_w(int state) override;

private:
	u8 buttons_r();

	// IO ports
	required_ioport m_buttons_io;
	required_ioport m_turbo_io;

	// internal states
	u8 m_counter; // Turbo rate counter, connected on 74xx163 QB and QC.
	bool m_prev_clr; // previous CLR pin state
};



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pce_joypad2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_joypad2 );
}


ioport_constructor pce_joypad2_turbo_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_joypad2_turbo );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_joypad2_device - constructor
//-------------------------------------------------

pce_joypad2_device::pce_joypad2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_muxer(*this, "mux")
{
}

pce_joypad2_device::pce_joypad2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pce_joypad2_device(mconfig, PCE_JOYPAD2, tag, owner, clock)
{
}


//-------------------------------------------------
//  pce_joypad2_turbo_device - constructor
//-------------------------------------------------

pce_joypad2_turbo_device::pce_joypad2_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pce_joypad2_device(mconfig, PCE_JOYPAD2_TURBO, tag, owner, clock),
	m_buttons_io(*this, "BUTTONS"),
	m_turbo_io(*this, "TURBO"),
	m_counter(0),
	m_prev_clr(false)
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void pce_joypad2_device::device_add_mconfig(machine_config &config)
{
	LS157(config, m_muxer);
	m_muxer->a_in_callback().set_ioport("BUTTONS");
	m_muxer->b_in_callback().set_ioport("DIRECTION");
}


void pce_joypad2_turbo_device::device_add_mconfig(machine_config &config)
{
	pce_joypad2_device::device_add_mconfig(config);
	m_muxer->a_in_callback().set(FUNC(pce_joypad2_turbo_device::buttons_r));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_joypad2_device::device_start()
{
}


void pce_joypad2_turbo_device::device_start()
{
	pce_joypad2_device::device_start();
	save_item(NAME(m_counter));
	save_item(NAME(m_prev_clr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pce_joypad2_turbo_device::device_reset()
{
	m_counter = 0;
	m_prev_clr = false;
}


//-------------------------------------------------
//  peripheral_r - joypad read
//-------------------------------------------------

u8 pce_joypad2_device::peripheral_r()
{
	return m_muxer->output_r();
}


//-------------------------------------------------
//  sel_w - MUXer select pin write
//-------------------------------------------------

void pce_joypad2_device::sel_w(int state)
{
	m_muxer->select_w(state);
}


//-------------------------------------------------
//  clr_w - MUXer strobe pin write
//-------------------------------------------------

void pce_joypad2_device::clr_w(int state)
{
	m_muxer->strobe_w(state);
}


void pce_joypad2_turbo_device::clr_w(int state)
{
	pce_joypad2_device::clr_w(state);
	if ((!m_prev_clr) && state) // rising edge, connected to 74xx163 clock pin
		m_counter = (m_counter + 1) & 7; // QA, QD pin not connected

	m_prev_clr = state;
}


//-------------------------------------------------
//  buttons_r - read button with autofire counter
//-------------------------------------------------

u8 pce_joypad2_turbo_device::buttons_r()
{
	u8 ret = m_buttons_io->read() & 0xf;
	const u8 turbo = m_turbo_io->read() & 0xf;
	for (int i = 0; i < 2; i++)
	{
		const u8 enable_bit = 1 + (i << 1);
		const u8 rate_bit = (i << 1);
		if (BIT(turbo, enable_bit)) // enable autofire
		{
			if (BIT(turbo, rate_bit)) // Fast
			{
				if (BIT(m_counter, 1)) // QB pin from 74xx163
					ret |= (1 << i);
			}
			else // Slow
			{
				if (BIT(m_counter, 2)) // QC pin from 74xx163
					ret |= (1 << i);
			}
		}
	}
	return ret;
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(PCE_JOYPAD2,       device_pce_control_port_interface, pce_joypad2_device,       "pce_joypad2",       "NEC PC Engine Pad")
DEFINE_DEVICE_TYPE_PRIVATE(PCE_JOYPAD2_TURBO, device_pce_control_port_interface, pce_joypad2_turbo_device, "pce_joypad2_turbo", "NEC PC Engine/TurboGrafx-16 2 Button Joypad")
