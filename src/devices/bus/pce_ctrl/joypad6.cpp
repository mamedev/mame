// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 6 button joypad emulation

    Based on SMS controller port emulation (devices\bus\sms_ctrl\*.*)
    by Fabio Priuli,
    PC engine emulation (mame\*\pce.*)
    by Charles MacDonald, Wilbert Pol, Angelo Salese

    There's 2 officially licensed 6 button joypad models:

    NEC Avenue Pad 6 (NAPD-1002)
    - Supports autofire for Button I, II, Run (slow motion)

    NEC Arcade Pad 6 (PCE-TP1)
    - Supports autofire for Button I-VI, Bundled in PC Engine Duo-RX

    TODO:
    - Needs verifications for Part numbers
    - Verify DTC114Y function for Slow motion in Avenue Pad 6

**********************************************************************/

#include "emu.h"
#include "joypad6.h"

#include "machine/74157.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pce_joypad6_base_device

class pce_joypad6_base_device : public device_t,
							public device_pce_control_port_interface
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(joypad_mode_changed);

protected:
	// construction/destruction
	pce_joypad6_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_pce_control_port_interface overrides
	virtual u8 peripheral_r() override;
	virtual void sel_w(int state) override;
	virtual void clr_w(int state) override;

	// button handlers
	void buttonset_update();

	// devices
	required_device_array<ls157_device, 3> m_muxer;

	// IO ports
	required_ioport m_joypad_mode;

	// internal states
	u8 m_counter; // buttonset select, autofire counter (74xx163 QA-QB pin)
	bool m_prev_clr; // previous CLR pin state
};


// ======================> pce_avenue_pad_6_device

class pce_avenue_pad_6_device : public pce_joypad6_base_device
{
public:
	// construction/destruction
	pce_avenue_pad_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// construction/destruction
	pce_avenue_pad_6_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	template<unsigned Buttonset> u8 buttons_r();

	// IO ports
	required_ioport_array<2> m_buttons_io;
	required_ioport m_turbo_io;
};


// ======================> pce_arcade_pad_6_device

class pce_arcade_pad_6_device : public pce_avenue_pad_6_device
{
public:
	// construction/destruction
	pce_arcade_pad_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};



INPUT_PORTS_START( pce_joypad6 )
	// Action button order on original pad is bottom row: III, II, I, and top row: IV, V, VI
	PORT_START("BUTTONS_0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("%p Button I")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("%p Button II")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("%p Select")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("%p Run")

	PORT_START("DIRECTION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY

	PORT_START("BUTTONS_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("%p Button III")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("%p Button IV")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("%p Button V")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("%p Button VI")

	PORT_START("JOY_MODE")
	PORT_CONFNAME( 0x01, 0x00, "Joypad Mode" ) PORT_CHANGED_MEMBER(DEVICE_SELF, pce_joypad6_base_device, joypad_mode_changed, 0)
	PORT_CONFSETTING( 0x00, "2-buttons mode" ) // A at avenue pad 6
	PORT_CONFSETTING( 0x01, "6-buttons mode" ) // B at avenue pad 6
INPUT_PORTS_END


static INPUT_PORTS_START( pce_avenue_pad_6 )
	PORT_INCLUDE( pce_joypad6 )

	PORT_START("TURBO")
	PORT_CONFNAME( 0x01, 0x00, "Button I Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Button II Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_CONFNAME( 0x08, 0x00, "Slow motion" ) // TODO: 74xx163 QB pin is connected with DTC114Y for Slow motion (Run button)
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT(      0xf4, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( pce_arcade_pad_6 )
	PORT_INCLUDE( pce_joypad6 )

	PORT_START("TURBO")
	PORT_CONFNAME( 0x01, 0x00, "Button I Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x00, "Button II Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT(      0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x10, 0x00, "Button III Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x10, DEF_STR( On ) )
	PORT_CONFNAME( 0x20, 0x00, "Button IV Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x20, DEF_STR( On ) )
	PORT_CONFNAME( 0x40, 0x00, "Button V Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x40, DEF_STR( On ) )
	PORT_CONFNAME( 0x80, 0x00, "Button VI Turbo" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor pce_avenue_pad_6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_avenue_pad_6 );
}


ioport_constructor pce_arcade_pad_6_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pce_arcade_pad_6 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_joypad6_base_device - constructor
//-------------------------------------------------

pce_joypad6_base_device::pce_joypad6_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pce_control_port_interface(mconfig, *this),
	m_muxer(*this, "mux_%u", 0U),
	m_joypad_mode(*this, "JOY_MODE"),
	m_counter(0),
	m_prev_clr(false)
{
}


//-------------------------------------------------
//  pce_avenue_pad_6_device - constructor
//-------------------------------------------------

pce_avenue_pad_6_device::pce_avenue_pad_6_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	pce_joypad6_base_device(mconfig, type, tag, owner, clock),
	m_buttons_io(*this, "BUTTONS_%u", 0U),
	m_turbo_io(*this, "TURBO")
{
}


pce_avenue_pad_6_device::pce_avenue_pad_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pce_avenue_pad_6_device(mconfig, PCE_AVENUE_PAD_6, tag, owner, clock)
{
}


//-------------------------------------------------
//  pce_arcade_pad_6_device - constructor
//-------------------------------------------------

pce_arcade_pad_6_device::pce_arcade_pad_6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pce_avenue_pad_6_device(mconfig, PCE_ARCADE_PAD_6, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void pce_joypad6_base_device::device_add_mconfig(machine_config &config)
{
	LS157(config, m_muxer[0]);
	m_muxer[0]->a_in_callback().set_ioport("BUTTONS_0");
	m_muxer[0]->b_in_callback().set_ioport("DIRECTION");

	LS157(config, m_muxer[1]);
	m_muxer[1]->a_in_callback().set_ioport("BUTTONS_1");
	m_muxer[1]->b_in_callback().set_constant(0x0); // 6-button pad header

	LS157(config, m_muxer[2]);
	m_muxer[2]->a_in_callback().set(m_muxer[0], FUNC(ls157_device::output_r));
	m_muxer[2]->b_in_callback().set(m_muxer[1], FUNC(ls157_device::output_r));
}


void pce_avenue_pad_6_device::device_add_mconfig(machine_config &config)
{
	pce_joypad6_base_device::device_add_mconfig(config);
	m_muxer[0]->a_in_callback().set(FUNC(pce_avenue_pad_6_device::buttons_r<0>));
	m_muxer[1]->a_in_callback().set(FUNC(pce_avenue_pad_6_device::buttons_r<1>));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_joypad6_base_device::device_start()
{
	save_item(NAME(m_counter));
	save_item(NAME(m_prev_clr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pce_joypad6_base_device::device_reset()
{
	m_counter = 0;
	m_prev_clr = false;
}


//-------------------------------------------------
//  peripheral_r - joypad read
//-------------------------------------------------

u8 pce_joypad6_base_device::peripheral_r()
{
	return m_muxer[2]->output_r();
}


//-------------------------------------------------
//  sel_w - MUXer select pin write
//-------------------------------------------------

void pce_joypad6_base_device::sel_w(int state)
{
	m_muxer[0]->select_w(state);
	m_muxer[1]->select_w(state);
}


//-------------------------------------------------
//  clr_w - MUXer strobe pin write, toggle button
//  set and autofire control
//-------------------------------------------------

void pce_joypad6_base_device::clr_w(int state)
{
	m_muxer[0]->strobe_w(state);
	m_muxer[1]->strobe_w(state);
	if ((!m_prev_clr) && state) // rising edge, connected to 74xx163 clock pin
		m_counter = (m_counter + 1) & 0x3; // Toggle buttons/autofire, connected to 74xx163; QC, QD pin not used

	m_prev_clr = state;
	buttonset_update();
}


//-------------------------------------------------
//  buttonset_update - toggle buttonsets
//-------------------------------------------------

void pce_joypad6_base_device::buttonset_update()
{
	m_muxer[2]->select_w((m_joypad_mode->read() & 1) ? BIT(m_counter, 0) : 0);
}


//-------------------------------------------------
//  joypad_mode_changed
//-------------------------------------------------

INPUT_CHANGED_MEMBER(pce_joypad6_base_device::joypad_mode_changed)
{
	buttonset_update();
}


//-------------------------------------------------
//  buttons_r - read button with autofire counter
//-------------------------------------------------

template<unsigned Buttonset>
u8 pce_avenue_pad_6_device::buttons_r()
{
	u8 ret = m_buttons_io[Buttonset]->read() & 0xf;
	const u8 turbo = BIT(m_turbo_io->read(), Buttonset << 2, 4);
	for (int i = 0; i < 4; i++)
	{
		if (BIT(turbo, i)) // enable autofire
		{
			if (BIT(m_counter, 1)) // QB pin from 74xx163
				ret |= (1 << i);
		}
	}
	return ret;
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(PCE_AVENUE_PAD_6, device_pce_control_port_interface, pce_avenue_pad_6_device, "pce_avenue_pad_6", "NEC Avenue Pad 6")
DEFINE_DEVICE_TYPE_PRIVATE(PCE_ARCADE_PAD_6, device_pce_control_port_interface, pce_arcade_pad_6_device, "pce_arcade_pad_6", "NEC Arcade Pad 6")
