// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  ** subclass of hh_sm510_state (includes/hh_sm510.h, drivers/hh_sm510.cpp) **

  Tiger R-Zone driver

  This is a backwards console, the heart of the machine is the cartridge.
  The console houses the controller, speaker, power, backlight, and a
  polarizer filter for the screen. The cartridge has the MCU, optional
  sound ROM, and the LCD screen in a translucent window.

  Console family:

  1995: R-Zone HeadGear: Wearable headset, controller is separate,
  red-on-black screen is reflected in front of the right eye.
  1996: R-Zone SuperScreen: Handheld console, inverted filter(aka black
  LCD segments), optional background sheet as with standalone handhelds.
  1997: R-Zone X.P.G - Xtreme Pocket Game: Handheld version of HeadGear.
  1997: R-Zone DataZone: PDA with a built-in SuperScreen.

  TODO:
  - support for SuperScreen. SVG colors will need to be inverted, or maybe
    with artwork or HLSL?
  - add DataZone, will get its own driver

***************************************************************************/

#include "emu.h"
#include "includes/hh_sm510.h"
#include "cpu/sm510/sm510.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "rzone.lh"

class rzone_state : public hh_sm510_state
{
public:
	rzone_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag),
		m_led_out(*this, "led"),
		m_led_off(*this, "led_off")
	{ }

	output_finder<> m_led_out;
	required_device<timer_device> m_led_off;

	int m_led_pin;
	int m_sctrl;
	int m_sclock;

	TIMER_DEVICE_CALLBACK_MEMBER(led_off_callback) { m_led_out = m_led_pin ? 1 : 0; }
	DECLARE_WRITE_LINE_MEMBER(led_w);
	DECLARE_WRITE_LINE_MEMBER(audio_w);
	DECLARE_WRITE_LINE_MEMBER(sctrl_w);
	DECLARE_WRITE_LINE_MEMBER(sclock_w);
	DECLARE_READ_LINE_MEMBER(sdata_r);

	DECLARE_WRITE8_MEMBER(t1_write_r);
	DECLARE_WRITE8_MEMBER(t1_write_s);
	virtual DECLARE_READ8_MEMBER(input_r) override;

protected:
	virtual void machine_start() override;
};


// machine start

void rzone_state::machine_start()
{
	hh_sm510_state::machine_start();

	// resolve handlers
	m_led_out.resolve();

	// zerofill
	m_led_pin = 0;
	m_sctrl = 0;
	m_sclock = 0;

	// register for savestates
	save_item(NAME(m_led_pin));
	save_item(NAME(m_sctrl));
	save_item(NAME(m_sclock));
}


/***************************************************************************

  I/O

***************************************************************************/

// console

WRITE_LINE_MEMBER(rzone_state::led_w)
{
	// LED: enable backlight
	if (state)
		m_led_out = 1;

	// delay led off to prevent flickering
	if (!state && m_led_pin)
		m_led_off->adjust(attotime::from_msec(30));

	m_led_pin = state;
}

WRITE_LINE_MEMBER(rzone_state::audio_w)
{
	// Audio: speaker out
	m_speaker->level_w(state ? 1 : 0);
}

WRITE_LINE_MEMBER(rzone_state::sctrl_w)
{
	// SCTRL: 74165 SH/LD: reload inputs while low
	if (!state || !m_sctrl)
		m_inp_mux = m_inp_matrix[0]->read();

	m_sctrl = state;
}

WRITE_LINE_MEMBER(rzone_state::sclock_w)
{
	// SCLOCK: 74165 CLK: shift inputs on rising edge when 74165 SH/LD is high
	if (m_sctrl && !m_sclock && state)
		m_inp_mux >>= 1;

	m_sclock = state;
}

READ_LINE_MEMBER(rzone_state::sdata_r)
{
	// SDATA: 74165 Q
	sctrl_w(m_sctrl); // reload inputs if needed
	return m_inp_mux & 1;
}


// cartridge type 1: simple SM510

WRITE8_MEMBER(rzone_state::t1_write_r)
{
	// R1: Audio
	audio_w(data & 1);

	// R2: SCTRL
	sctrl_w(data >> 1 & 1);
}

WRITE8_MEMBER(rzone_state::t1_write_s)
{
	// S1: LED
	led_w(data & 1);

	// S2: SCLOCK
	sclock_w(data >> 1 & 1);
}

READ8_MEMBER(rzone_state::input_r)
{
	// K1: SDATA
	return sdata_r();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( rzone )
	PORT_START("IN.0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // A
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // B
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // C
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // D
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_NAME("Sound")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_POWER_OFF )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Pause")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( rzindy500 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz) // no external XTAL
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(rzone_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(rzone_state, t1_write_s))
	MCFG_SM510_WRITE_R_CB(WRITE8(rzone_state, t1_write_r))
	MCFG_SM510_R_DIRECT_CONTROL(true)

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1425, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1425-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD("led_off", rzone_state, led_off_callback)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_rzone)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rzindy500 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_22", 0x0000, 0x1000, CRC(99a746d0) SHA1(64264499d45a566fa9a0801c20e7fa27eac18da6) )

	ROM_REGION( 533407, "svg", 0)
	ROM_LOAD( "rzindy500.svg", 0, 533407, CRC(07f72e35) SHA1(034972c1255f8899b53a94063d6c66bdef089ce9) )
ROM_END


//    YEAR  NAME       PARENT  COMP MACHINE    INPUT      STATE        INIT  COMPANY, FULLNAME, FLAGS
CONS( 1995, rzindy500, 0,        0, rzindy500, rzone,     rzone_state,    0, "Tiger Electronics (licensed from Sega)", "R-Zone: Indy 500", MACHINE_SUPPORTS_SAVE )
