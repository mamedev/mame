// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton, Sean Riddle
/***************************************************************************

Entex Select-A-Game Machine, handheld game console.
Technically, the main unit is the peripheral(buttons, display, speaker, power),
and the cartridge holds the MCU(processor, ROM, RAM).

Hardware notes:
- cyan/red VFD display Futaba DM-16Z + cyan VFD 9-digit panel Futaba 9-ST-11A 1F
- 1-bit sound, two 7-button control panels attached to each side
- edge connector to cartridge, MCU on cartridge (see below for each MCU type)

Games released, MCU: (*denotes undumped)
- Baseball 4       - HD38800
- *Basketball 3    - ?
- Football 4       - TMS1670
- *Pac-Man 2       - HD38800?
- Pinball          - HD38800
- *Space Invader 2 - HD38800 - is dumped, but need redump

Battleship and Turtles were announced but unreleased.
A 2nd version of the console was also announced, called Table Top Game Machine,
but Entex didn't release it. Their next console was the Adventure Vision.

MAME external artwork is recommended for the per-game VFD overlays. For internal
artwork, remember that the orientation can be rotated in the video options.
By default, the "visitor" side is at the bottom. This is how most of the games
are played, Space Invader 2 is an exception.

TODO:
- add softwarelist? impractical with different MCUs, and no CPU inside console
- add the rest of the games

***************************************************************************/

#include "emu.h"
#include "cpu/hmcs40/hmcs40.h"
#include "cpu/tms1000/tms1400.h"
#include "video/pwm.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "sag.lh"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void shared(machine_config &config);

protected:
	virtual void machine_start() override;

	void update_display();
	u8 input_r();
	void speaker_w(int state);

	u16 m_grid = 0;
	u16 m_plate = 0;

private:
	required_device<pwm_display_device> m_display;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<6> m_inputs;
};

void base_state::machine_start()
{
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

class hmcs40_state : public base_state
{
public:
	hmcs40_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sag(machine_config &config);

private:
	required_device<hmcs40_cpu_device> m_maincpu;

	DECLARE_WRITE8_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_d);
	DECLARE_READ16_MEMBER(read_d);
};

class tms1k_state : public base_state
{
public:
	tms1k_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sag(machine_config &config);

private:
	required_device<tms1k_base_device> m_maincpu;

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};



/******************************************************************************
    I/O
******************************************************************************/

// main unit

void base_state::update_display()
{
	// grid 0-7 are the 'pixels'
	m_display->matrix_partial(0, 8, m_grid, m_plate, false);

	// grid 8-13 are 7segs
	u8 seg = bitswap<7>(m_plate,4,5,6,7,8,9,10);
	m_display->matrix_partial(8, 6, m_grid >> 8, seg);
}

u8 base_state::input_r()
{
	u8 data = 0;

	// grid 1-6 double as input mux
	for (int i = 0; i < 6; i++)
		if (BIT(m_grid, i + 1))
			data |= m_inputs[i]->read();

	return data;
}

void base_state::speaker_w(int state)
{
	m_speaker->level_w(state);
}


// cartridge type 1: HD38800

WRITE8_MEMBER(hmcs40_state::write_r)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

WRITE16_MEMBER(hmcs40_state::write_d)
{
	// D0: speaker out
	speaker_w(data & 1);

	// D1-D12: vfd grid (10 and 11 unused)
	m_grid = bitswap<14>(data,9,10,0,0,11,12,1,2,3,4,5,6,7,8) & 0x33ff;
	update_display();
}

READ16_MEMBER(hmcs40_state::read_d)
{
	// D13-D15: multiplexed inputs
	return input_r() << 13;
}


// cartridge type 2: TMS1670

WRITE16_MEMBER(tms1k_state::write_r)
{
	// R0: speaker out
	speaker_w(data & 1);

	// R1-R12: vfd grid (0 and 7 unused)
	// R13,R14: vfd plate 3,2
	m_grid = bitswap<14>(data,7,8,9,10,11,12,0,1,2,3,4,5,6,0) & 0x3f7e;
	m_plate = (m_plate & 0xff0) | bitswap<2>(data,13,14) << 2;
	update_display();
}

WRITE16_MEMBER(tms1k_state::write_o)
{
	// O0-O7: vfd plate 4-11
	m_plate = (m_plate & 0xf) | data << 4;
	update_display();
}

READ8_MEMBER(tms1k_state::read_k)
{
	// K1-K4: multiplexed inputs
	return input_r();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sag ) // P1 = Visitor (left side), P2 = Home (right side)
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Button 6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Button 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Button 7")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Button 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Button 5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Button 7")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Button 2")
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x00) // demo

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("P1 Button 1")
	PORT_CONFNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Button 4")
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x01) // 1 player

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("P1 Button 3")
	PORT_CONFNAME( 0x04, 0x04, "Game" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("FAKE") // shared IN.2/IN.4
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "Demo" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void base_state::shared(machine_config &config)
{
	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(8+6, 14);
	m_display->set_segmask(0x3f00, 0x7f);
	config.set_default_layout(layout_sag);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void hmcs40_state::sag(machine_config &config)
{
	/* basic machine hardware */
	HD38800(config, m_maincpu, 450000); // approximation
	m_maincpu->write_r<0>().set(FUNC(hmcs40_state::write_r));
	m_maincpu->write_r<1>().set(FUNC(hmcs40_state::write_r));
	m_maincpu->write_r<2>().set(FUNC(hmcs40_state::write_r));
	m_maincpu->write_r<3>().set(FUNC(hmcs40_state::write_r));
	m_maincpu->write_d().set(FUNC(hmcs40_state::write_d));
	m_maincpu->read_d().set(FUNC(hmcs40_state::read_d));

	shared(config);
}

void tms1k_state::sag(machine_config &config)
{
	/* basic machine hardware */
	TMS1670(config, m_maincpu, 375000); // approximation - RC osc. R=47K, C=47pF
	m_maincpu->k().set(FUNC(tms1k_state::read_k));
	m_maincpu->r().set(FUNC(tms1k_state::write_r));
	m_maincpu->o().set(FUNC(tms1k_state::write_o));

	shared(config);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( sag_sinv2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "inv2_hd38800a31", 0x0000, 0x1000, BAD_DUMP CRC(29c8c100) SHA1(41cd413065659c6d7d5b2408de2ca6d51c49629a) )
	ROM_CONTINUE( 0x1e80, 0x0100 )
ROM_END

ROM_START( sag_baseb4 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b-b5_hd38800a37", 0x0000, 0x1000, CRC(64852bd5) SHA1(fb1c24ca43934ceb6fc35ac7c35b71e6e843dbc5) )
	ROM_CONTINUE( 0x1e80, 0x0100 )
ROM_END

ROM_START( sag_pinb )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "pinb_hd38800a38", 0x0000, 0x1000, CRC(6e53a56b) SHA1(13f057eab2e4cfbb3ef1247a041abff15ae727c9) )
	ROM_CONTINUE( 0x1e80, 0x0100 )
ROM_END

ROM_START( sag_footb4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ftba_mp7573", 0x0000, 0x1000, CRC(b17dd9e3) SHA1(9c9e7a56643233ef2adff7b68a6df19e6ca176c2) ) // die label TMS1400, MP7573

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_sag_fb4_output.pla", 0, 557, CRC(f15dc6a1) SHA1(ee11a64037895ac566e902b6b590ff62a7f703b0) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME        PARENT CMP MACHINE INPUT  CLASS         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1981, sag_sinv2,  0,      0, sag,    sag,   hmcs40_state, empty_init, "Entex", "Select-A-Game Machine: Space Invader 2", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING ) // suspect bad dump
CONS( 1981, sag_baseb4, 0,      0, sag,    sag,   hmcs40_state, empty_init, "Entex", "Select-A-Game Machine: Baseball 4", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
CONS( 1981, sag_pinb,   0,      0, sag,    sag,   hmcs40_state, empty_init, "Entex", "Select-A-Game Machine: Pinball", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )

CONS( 1981, sag_footb4, 0,      0, sag,    sag,   tms1k_state,  empty_init, "Entex", "Select-A-Game Machine: Football 4", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
