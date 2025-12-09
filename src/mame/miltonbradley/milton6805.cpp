// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Milton Bradley Milton

This is the talking tabletop game, not the chess computer with the same name.

Game 1: Match beginning of a phrase(red button) with end of phrase(yellow button).
Game 2: Same as game 1, but all in one turn.
Game 3: Press phrase end buttons, memorize them, press Go and match them.

Hardware is an odd combination: MC6805P2 MCU, GI SP0250 speech + 2*TMC0430 GROM.
See patent 4326710 for detailed information, except MC6805 clocked from SP0250 3.12MHz
and GROM clocked by 3.12MHz/8=390kHz.

*******************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68705.h"
#include "machine/clock.h"
#include "machine/tmc0430.h"
#include "sound/sp0250.h"
#include "speaker.h"

// internal artwork
#include "milton.lh"

class milton_filter_device;


class milton_state : public driver_device
{
public:
	milton_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_grom(*this, "grom%u", 0),
		m_speech(*this, "sp0250"),
		m_filter(*this, "filter"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void milton(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(volume_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<m6805_hmos_device> m_maincpu;
	required_device_array<tmc0430_device, 2> m_grom;
	required_device<sp0250_device> m_speech;
	required_device<milton_filter_device> m_filter;
	required_ioport_array<5> m_inputs;

	u8 m_data = 0;
	u8 m_control = 0xff;

	void data_w(u8 data);
	u8 data_r();
	void control_w(u8 data);
	u8 control_r();
	u8 input_r();
};

void milton_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_control));
}



/*******************************************************************************
    LED Filter
*******************************************************************************/

class milton_filter_device : public device_t, public device_sound_interface
{
public:
	milton_filter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream = nullptr;
	output_finder<> m_led_out;
};

DEFINE_DEVICE_TYPE(MILTON_LED_FILTER, milton_filter_device, "milton_led_filter", "Milton LED Filter")


milton_filter_device::milton_filter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MILTON_LED_FILTER, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_led_out(*this, "led")
{ }

void milton_filter_device::device_start()
{
	m_stream = stream_alloc(1, 1, machine().sample_rate());
	m_led_out.resolve();
}

void milton_filter_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t level = 0;

	for (int i = 0; i < stream.samples(); i++)
		level += fabsf(stream.get(0, i));

	stream.copy(0, 0);

	if (stream.samples() > 0)
		level /= stream.samples();

	// 2 leds connected to the audio circuit
	const sound_stream::sample_t threshold = 1500.0 / 32768.0;
	m_led_out = (level > threshold) ? 1 : 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

void milton_state::data_w(u8 data)
{
	// TMC0430 + SP0250 data
	m_data = data;
}

u8 milton_state::data_r()
{
	if (machine().side_effects_disabled())
		return 0;

	// TMC0430 data
	u8 data = 0;
	m_grom[0]->readz(&data);
	m_grom[1]->readz(&data);
	return data;
}

void milton_state::control_w(u8 data)
{
	// d0-d4: input mux

	// d5: SP0250 data present
	if (~m_control & data & 0x20)
		m_speech->write(m_data);

	// d1: TMC0430 M
	// d3: TMC0430 MO
	// d7: TMC0430 GS
	for (int i = 0; i < 2; i++)
	{
		m_grom[i]->m_line(BIT(data, 1));
		m_grom[i]->mo_line(BIT(data, 3));
		m_grom[i]->gsq_line(BIT(~data, 7));
	}

	// write pending TMC0430 data
	if (m_control & ~data & 0x80 && ~data & 2)
	{
		m_grom[0]->write(m_data);
		m_grom[1]->write(m_data);
	}

	m_control = data;
}

u8 milton_state::control_r()
{
	if (machine().side_effects_disabled())
		return 0;

	// d6: SP0250 data request
	// other: no function (DDRB = 0xbf)
	return m_speech->drq_r() ? 0x40 : 0;
}

u8 milton_state::input_r()
{
	u8 data = 0;

	// d0-d3: multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (BIT(~m_control, i))
			data |= m_inputs[i]->read();

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( milton )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Red Button 7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Red Button 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Red Button 5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Red Button 4")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Red Button 3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Red Button 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Red Button 1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Purple Button 1")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Purple Button 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Purple Button 3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Go")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Score")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Reset")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Yellow Button 1") // starting at top, then clockwise
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Yellow Button 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Yellow Button 3")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Yellow Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Yellow Button 5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Yellow Button 6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Yellow Button 7")

	PORT_START("VOLUME")
	PORT_CONFNAME( 0x01, 0x00, "Volume" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(milton_state::volume_changed), 0)
	PORT_CONFSETTING(    0x01, "Low" )
	PORT_CONFSETTING(    0x00, "High" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(milton_state::volume_changed)
{
	m_filter->set_output_gain(0, newval ? 0.25 : 1.0);
}



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void milton_state::milton(machine_config &config)
{
	// basic machine hardware
	M6805P2(config, m_maincpu, 3.12_MHz_XTAL);
	m_maincpu->porta_w().set(FUNC(milton_state::data_w));
	m_maincpu->porta_r().set(FUNC(milton_state::data_r));
	m_maincpu->portb_w().set(FUNC(milton_state::control_w));
	m_maincpu->portb_r().set(FUNC(milton_state::control_r));
	m_maincpu->portc_r().set(FUNC(milton_state::input_r));

	TMC0430(config, m_grom[0], "groms", 0x0000, 0);
	TMC0430(config, m_grom[1], "groms", 0x2000, 1);

	clock_device &gromclock(CLOCK(config, "gromclock", 3.12_MHz_XTAL/8));
	gromclock.signal_handler().set(m_grom[0], FUNC(tmc0430_device::gclock_in));
	gromclock.signal_handler().append(m_grom[1], FUNC(tmc0430_device::gclock_in));

	config.set_default_layout(layout_milton);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SP0250(config, m_speech, 3.12_MHz_XTAL).add_route(0, m_filter, 1.0, 0);
	MILTON_LED_FILTER(config, m_filter).add_route(0, "speaker", 1.0);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( milton )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD("sc87008p_783-4043-001", 0x000, 0x800, CRC(b054dbea) SHA1(b5339c8170e773b68505c3d60dc75249a583d60a) )

	ROM_REGION( 0x4000, "groms", ROMREGION_ERASE00 )
	ROM_LOAD("4043-003", 0x0000, 0x1800, CRC(d95df757) SHA1(6723480866f6393d310e304ef3b61e3a319a7beb) )
	ROM_LOAD("4043-004", 0x2000, 0x1800, CRC(9ac929f7) SHA1(1a27d56fc49eb4e58ea3b5c58d7fbedc5a751592) )
ROM_END



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, milton, 0,      0,      milton,  milton, milton_state, empty_init, "Milton Bradley", "Electronic Milton", MACHINE_SUPPORTS_SAVE )
