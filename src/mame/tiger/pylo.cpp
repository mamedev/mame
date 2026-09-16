// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Frank Palazzolo
/*******************************************************************************

Tiger Punch Your Lights Out (model 7-571)

Hardware notes:
- PCB label: 7-571A
- TI TMS70C40 @ 4MHz, custom label but decap shows it's a TMS70C40F with
  ROM serial C61013, 4KB internal ROM is half empty
- 4 7segs, 4*6 other leds, piezo

There's also a version with a Toshiba TMP47C200.

*******************************************************************************/

#include "emu.h"

#include "cpu/tms7000/tms7000.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

#include "pylo.lh"


namespace {

class pylo_state : public driver_device
{
public:
	pylo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void pylo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<tms70c40_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<6> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_digit_data = 0;
	u8 m_led_data = 0;

	void update_display();
	u8 input_r();
	void input_w(u8 data);
	void digit_w(u8 data);
	void led_w(u8 data);
};

void pylo_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_digit_data));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void pylo_state::update_display()
{
	m_display->matrix(m_inp_mux >> 2, m_led_data << 7 | m_digit_data);
}

u8 pylo_state::input_r()
{
	u8 data = 0;

	// A0-A7: multiplexed inputs
	for (int i = 0; i < 6; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return data;
}

void pylo_state::input_w(u8 data)
{
	// B0-B5: input mux
	// B2-B5: digit/led select
	// B6,B7: N/C
	m_inp_mux = data & 0x3f;
	update_display();
}

void pylo_state::digit_w(u8 data)
{
	// C0-C6: digit segments
	// C7: N/C
	m_digit_data = ~data & 0x7f;
	update_display();
}

void pylo_state::led_w(u8 data)
{
	// D0-D5: led data
	m_led_data = ~data & 0x3f;
	update_display();

	// D6: N/C
	// D7: speaker out
	m_speaker->level_w(BIT(~data, 7));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( pylo )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_CONFNAME( 0x0e, 0x02, "Game" )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON4)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x0e, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x0e, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON9)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON10)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON11)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON12)

	PORT_START("IN.3")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_COCKTAIL
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_COCKTAIL

	PORT_START("IN.4")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_COCKTAIL
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_COCKTAIL
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_COCKTAIL

	PORT_START("IN.5")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_COCKTAIL
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_COCKTAIL
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_COCKTAIL
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void pylo_state::pylo(machine_config &config)
{
	// basic machine hardware
	TMS70C40(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->in_porta().set(FUNC(pylo_state::input_r));
	m_maincpu->out_portb().set(FUNC(pylo_state::input_w));
	m_maincpu->out_portc().set(FUNC(pylo_state::digit_w));
	m_maincpu->out_portd().set(FUNC(pylo_state::led_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 13);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_pylo);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( pylo )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("1-7571a1-2.u1", 0x0000, 0x1000, CRC(0cb01059) SHA1(68a422cef6513e8eed2d83d492e2be11d049d183) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1988, pylo, 0,      0,      pylo,    pylo,  pylo_state, empty_init, "Tiger", "Punch Your Lights Out", MACHINE_SUPPORTS_SAVE )
