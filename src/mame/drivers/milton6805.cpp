// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/******************************************************************************

Milton Bradley Milton

This is the talking tabletop game, not the chess computer with the same name.

Hardware is an odd combination: MC6805P2 MCU, GI SP0250 speech + 2*TMC0430 GROM.
See patent 4326710 for detailed information, except MC6805 clocked from SP0250 3.12MHz
and GROM clocked by 3.12MHz/8=390kHz.

TODO:
- 2 leds connected to audio out

******************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68705.h"
#include "machine/clock.h"
#include "machine/tmc0430.h"
#include "sound/sp0250.h"
#include "speaker.h"


namespace {

class milton_state : public driver_device
{
public:
	milton_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_grom(*this, "grom%u", 0),
		m_speech(*this, "sp0250"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void milton(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<m6805_hmos_device> m_maincpu;
	required_device_array<tmc0430_device, 2> m_grom;
	required_device<sp0250_device> m_speech;
	required_ioport_array<5> m_inputs;

	u8 m_data;
	u8 m_control;

	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_READ8_MEMBER(input_r);
};

void milton_state::machine_start()
{
	// zerofill
	m_data = 0;
	m_control = 0xff;

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_control));
}


/******************************************************************************
    I/O
******************************************************************************/

WRITE8_MEMBER(milton_state::data_w)
{
	// TMC0430 + SP0250 data
	m_data = data;
}

READ8_MEMBER(milton_state::data_r)
{
	if (machine().side_effects_disabled())
		return 0;

	// TMC0430 data
	u8 data = 0xff;
	m_grom[0]->readz(&data);
	m_grom[1]->readz(&data);
	return data;
}

WRITE8_MEMBER(milton_state::control_w)
{
	// d0-d4: input mux

	// d5: SP0250 data present
	if (~m_control & data & 0x20)
		m_speech->write(space, 0, m_data);

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

READ8_MEMBER(milton_state::control_r)
{
	if (machine().side_effects_disabled())
		return 0;

	// d6: SP0250 data request
	// other: no function (DDRB = 0xbf)
	return m_speech->drq_r() ? 0x40 : 0;
}

READ8_MEMBER(milton_state::input_r)
{
	u8 data = 0;

	// d0-d3: multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (BIT(~m_control, i))
			data |= m_inputs[i]->read();

	return ~data;
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( milton )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Red Button 7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Red Button 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Red Button 5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Red Button 4")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Red Button 3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Red Button 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Red Button 1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Purple Button 1")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Purple Button 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Purple Button 3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Go")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Score")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Reset")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Yellow Button 1") // starting at top, then clockwise
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Yellow Button 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Yellow Button 3")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Yellow Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Yellow Button 5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Yellow Button 6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Yellow Button 7")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void milton_state::milton(machine_config &config)
{
	/* basic machine hardware */
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

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	SP0250(config, m_speech, 3.12_MHz_XTAL).add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( milton )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD("sc87008p_783-4043-001", 0x000, 0x800, CRC(b054dbea) SHA1(b5339c8170e773b68505c3d60dc75249a583d60a) )

	ROM_REGION( 0x4000, "groms", ROMREGION_ERASEFF )
	ROM_LOAD("4043-003", 0x0000, 0x1800, CRC(d95df757) SHA1(6723480866f6393d310e304ef3b61e3a319a7beb) )
	ROM_LOAD("4043-004", 0x2000, 0x1800, CRC(9ac929f7) SHA1(1a27d56fc49eb4e58ea3b5c58d7fbedc5a751592) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME    PARENT CMP MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1980, milton, 0,      0, milton,  milton, milton_state, empty_init, "Milton Bradley", "Electronic Milton", MACHINE_SUPPORTS_SAVE )
