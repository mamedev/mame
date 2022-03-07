// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton, Sean Riddle
/***************************************************************************

NEC uCOM4 MCU tabletops/handhelds or other simple devices,
most of them (emulated ones) are VFD electronic games/toys.

known chips:

  serial  device   etc.
----------------------------------------------------------------
  055     uPD546C  1978, Fidelity Checker Challenger (CR) -> fidel_checkc2.cpp

 @017     uPD552C  1979, Bambino UFO Master-Blaster Station (ET-02)
 @042     uPD552C  1980, Tomy Cosmic Combat (TN-??)
 @043     uPD552C  1979, Bambino Kick The Goal Soccer (ET-10)
 *044     uPD552C  1979, Bambino Lucky Puck Ice Hockey (ET-08)
 @048     uPD552C  1980, Tomy Tennis (TN-04)
 @049     uPD552C  1981, Bambino Safari (ET-11)
 @054     uPD552C  1980, Epoch Invader From Space

 @031     uPD553C  1979, Bambino Superstar Football (ET-03)
 @049     uPD553C  1979, Mego Mini-Vid: Break Free
 @055     uPD553C  1980, Bambino Space Laser Fight (ET-12)
 *073     uPD553C  1980, Sony ST-J75 FM Stereo Tuner
 @080     uPD553C  1980, Epoch Electronic Football
 *102     uPD553C  1981, Bandai Block Out
 @153     uPD553C  1981, Epoch Galaxy II
 @160     uPD553C  1982, Tomy Pac Man (TN-08)
 *167     uPD553C  1982, Sony SL models (betamax) (have dump)
 @170     uPD553C  1982, Bandai Crazy Climber
 @192     uPD553C  1982, Tomy Scramble (TN-10)
 @202     uPD553C  1982, Epoch Astro Command
 @206     uPD553C  1982, Epoch Dracula
 *207     uPD553C  1982, Sony SL-J30 (tape/cd deck)
 @209     uPD553C  1982, Tomy Caveman (TN-12)
 @258     uPD553C  1984, Tomy Alien Chase (TN-16)
 *296     uPD553C  1984, Epoch Computer Beam Gun Professional

 @511     uPD557LC 1980, Takatoku Toys Game Robot 9/Mego Fabulous Fred
 @512     uPD557LC 1980, Castle Toy Tactix
 @513     uPD557LC 1980, Castle Toy Name That Tune

 @060     uPD650C  1979, Mattel Computer Gin
  085     uPD650C  1980, Roland TR-808 -> roland_tr808.cpp
 *127     uPD650C  198?, Sony OA-S1100 Typecorder (subcpu, have dump)
  128     uPD650C  1981, Roland TR-606 -> roland_tr606.cpp
  133     uPD650C  1982, Roland TB-303 -> roland_tb303.cpp

  (* means undumped unless noted, @ denotes it's in this driver)


Commonly used VFD(vacuum fluorescent display) are by NEC or Futaba.

NEC FIP9AM20T (example, Epoch Astro Command)
       grcss

FIP = fluorescent indicator panel
g = number of grids
r = revision of the VFD
c = custom display
s = unique display part number

============================================================================

ROM source notes when dumped from another publisher, but confident it's the same:
- astrocmd: Tandy Astro Command
- caveman: Tandy Caveman
- grobot9: Mego Fabulous Fred

TODO:
- get rid of hardcoded color overlay from SVGs, use MAME internal artwork

***************************************************************************/

#include "emu.h"

#include "cpu/ucom4/ucom4.h"
#include "video/pwm.h"
#include "video/hlcd0515.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

// internal artwork (complete)
#include "ctntune.lh" // clickable
#include "efball.lh"
#include "grobot9.lh" // clickable
#include "mcompgin.lh"
#include "mvbfree.lh"
#include "tactix.lh" // clickable

// internal artwork (bezel overlay)
#include "tmtennis.lh"

//#include "hh_ucom4_test.lh" // common test-layout - no svg artwork(yet), use external artwork


class hh_ucom4_state : public driver_device
{
public:
	hh_ucom4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(single_interrupt_line);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<ucom4_cpu_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<6> m_inputs; // max 6

	// misc common
	u8 m_port[9] = { };             // MCU port A-I write data (optional)
	u8 m_int = 0;                   // MCU INT pin state
	u16 m_inp_mux = 0;              // multiplexed inputs mask

	u32 m_grid = 0;                 // VFD current row data
	u32 m_plate = 0;                // VFD current column data

	u8 read_inputs(int columns);
	void refresh_interrupts(void);
	void set_interrupt(int state);

	enum
	{
		PORTA = 0,
		PORTB,
		PORTC,
		PORTD,
		PORTE,
		PORTF,
		PORTG,
		PORTH,
		PORTI
	};
};


// machine start/reset

void hh_ucom4_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_port));
	save_item(NAME(m_int));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_ucom4_state::machine_reset()
{
	refresh_interrupts();
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_ucom4_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}


// interrupt handling

void hh_ucom4_state::refresh_interrupts()
{
	m_maincpu->set_input_line(0, m_int ? ASSERT_LINE : CLEAR_LINE);
}

void hh_ucom4_state::set_interrupt(int state)
{
	state = state ? 1 : 0;

	if (state != m_int)
	{
		if (machine().phase() >= machine_phase::RESET)
			m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
		m_int = state;
	}
}

INPUT_CHANGED_MEMBER(hh_ucom4_state::single_interrupt_line)
{
	set_interrupt(newval);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Bambino UFO Master-Blaster Station (manufactured in Japan)
  * PCB label Emix Corp. ET-02
  * NEC uCOM-44 MCU, label EMIX D552C 017
  * cyan VFD display Emix-101, with blue color overlay

  This is Bambino's first game, it is not known if ET-01 exists. Emix Corp.
  wasn't initially a toy company, the first release was through Tomy. Emix
  created the Bambino brand afterwards. It is claimed to be the first
  computerized VFD game (true, unless TI Speak & Spell or M.E.M. Memoquiz
  from 1978 are considered VFD games)

  known releases:
  - Japan: "Missile Guerilla Warfare Maneuvers", published by Tomy
  - World: UFO Master-Blaster Station, published by Bambino

***************************************************************************/

class ufombs_state : public hh_ucom4_state
{
public:
	ufombs_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void ufombs(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	void speaker_w(u8 data);
};

// handlers

void ufombs_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,3,2,1,0,4,5,6,7,8);
	u16 plate = bitswap<16>(m_plate,15,14,13,12,11,7,10,6,9,5,8,4,0,1,2,3);
	m_display->matrix(grid, plate);
}

void ufombs_state::grid_w(offs_t offset, u8 data)
{
	// F,G,H0: vfd grid
	int shift = (offset - PORTF) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void ufombs_state::plate_w(offs_t offset, u8 data)
{
	// C,D012,I: vfd plate
	int shift = (offset == PORTI) ? 8 : (offset - PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void ufombs_state::speaker_w(u8 data)
{
	// E01: speaker out
	m_speaker->level_w(data & 3);
}

// config

static INPUT_PORTS_START( ufombs )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("IN.0", 0x0a, EQUALS, 0x00) // pad in the middle, pressed when joystick is centered
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("IN.1") // port B
	PORT_CONFNAME( 0x07, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void ufombs_state::ufombs(machine_config &config)
{
	// basic machine hardware
	NEC_D552(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(ufombs_state::plate_w));
	m_maincpu->write_d().set(FUNC(ufombs_state::plate_w));
	m_maincpu->write_e().set(FUNC(ufombs_state::speaker_w));
	m_maincpu->write_f().set(FUNC(ufombs_state::grid_w));
	m_maincpu->write_g().set(FUNC(ufombs_state::grid_w));
	m_maincpu->write_h().set(FUNC(ufombs_state::grid_w));
	m_maincpu->write_i().set(FUNC(ufombs_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(243, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 10);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ufombs )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-017", 0x0000, 0x0400, CRC(0e208cb3) SHA1(57db6566916c94325e2b67ccb94b4ea3b233487d) )

	ROM_REGION( 222395, "screen", 0)
	ROM_LOAD( "ufombs.svg", 0, 222395, CRC(ae9fb93f) SHA1(165ea78eee93c503dbd277a56c41e3c63c534e38) )
ROM_END





/***************************************************************************

  Bambino Superstar Football (manufactured in Japan)
  * PCB label Emix Corp. ET-03
  * NEC uCOM-43 MCU, label D553C 031
  * cyan VFD display Emix-102, with bezel

  The game was rereleased in 1982 as Classic Football (ET-0351), with an
  improved cyan/green/red VFD.

  Press the Kick button to start the game, an automatic sequence follows.
  Then choose a formation(A,B,C) and either pass the ball, and/or start
  running. For more information, refer to the official manual.

***************************************************************************/

class ssfball_state : public hh_ucom4_state
{
public:
	ssfball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void ssfball(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	u8 input_b_r();
};

// handlers

void ssfball_state::update_display()
{
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,11,7,3,12,17,13,18,16,14,15,10,9,8,0,1,2,4,5,6);
	m_display->matrix(m_grid, plate);
}

void ssfball_state::grid_w(offs_t offset, u8 data)
{
	// C,D(,E3): vfd grid 0-7(,8)
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void ssfball_state::plate_w(offs_t offset, u8 data)
{
	m_port[offset] = data;

	// E,F,G,H,I(not all!): vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// F3,G3: input mux + speaker
	m_inp_mux = (m_port[PORTF] >> 3 & 1) | (m_port[PORTG] >> 2 & 2);
	m_speaker->level_w(m_inp_mux);

	// E3: vfd grid 8
	if (offset == PORTE)
		grid_w(offset, data >> 3 & 1);
	else
		update_display();
}

u8 ssfball_state::input_b_r()
{
	// B: input port 2, where B3 is multiplexed
	return m_inputs[2]->read() | read_inputs(2);
}

// config

/* physical button layout and labels are like this:

    [A]    [B]    [C]    [PASS]  [KICK/
       ^FORMATION^                DISPLAY]

                                 [^]
                         [<>]
    (game lvl sw)                [v]
    1<---OFF--->2
*/

static INPUT_PORTS_START( ssfball )
	PORT_START("IN.0") // F3 port B3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Formation A")

	PORT_START("IN.1") // G3 port B3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Game Level" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("IN.2") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Kick/Display")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Formation C")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Formation B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) // multiplexed, handled in input_b_r

	PORT_START("IN.3") // port A
	PORT_BIT( 0x01, 0x01, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // left/right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Pass")

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
INPUT_PORTS_END

void ssfball_state::ssfball(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set_ioport("IN.3");
	m_maincpu->read_b().set(FUNC(ssfball_state::input_b_r));
	m_maincpu->write_c().set(FUNC(ssfball_state::grid_w));
	m_maincpu->write_d().set(FUNC(ssfball_state::grid_w));
	m_maincpu->write_e().set(FUNC(ssfball_state::plate_w));
	m_maincpu->write_f().set(FUNC(ssfball_state::plate_w));
	m_maincpu->write_g().set(FUNC(ssfball_state::plate_w));
	m_maincpu->write_h().set(FUNC(ssfball_state::plate_w));
	m_maincpu->write_i().set(FUNC(ssfball_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 482);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ssfball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-031", 0x0000, 0x0800, CRC(ff5d91d0) SHA1(9b2c0ae45f1e3535108ee5fef8a9010e00c8d5c3) )

	ROM_REGION( 331352, "screen", 0)
	ROM_LOAD( "ssfball.svg", 0, 331352, CRC(10cffb85) SHA1(c875f73a323d976088ffa1bc19f7bc865d4aac62) )
ROM_END

ROM_START( bmcfball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-031", 0x0000, 0x0800, CRC(ff5d91d0) SHA1(9b2c0ae45f1e3535108ee5fef8a9010e00c8d5c3) )

	ROM_REGION( 331352, "screen", 0)
	ROM_LOAD( "bmcfball.svg", 0, 331352, CRC(43fbed1e) SHA1(28160e14b0879cd4dd9dab770c52c98f316ab653) )
ROM_END





/***************************************************************************

  Bambino Kick The Goal Soccer
  * PCB label Emix Corp. ET-10/08 (PCB is for 2 possible games)
  * NEC uCOM-44 MCU, label D552C 043
  * cyan VFD display Emix-105, with bezel overlay

  Press the Display button twice to start the game. Action won't start until
  player 1 presses one of the directional keys. In 2-player mode, player 2
  controls the goalkeeper, defensive players are still controlled by the CPU.

***************************************************************************/

class bmsoccer_state : public hh_ucom4_state
{
public:
	bmsoccer_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void bmsoccer(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	u8 input_a_r();
};

// handlers

void bmsoccer_state::update_display()
{
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,11,7,3,12,17,13,18,16,14,15,8,4,0,9,5,1,10,6,2);
	m_display->matrix(m_grid, plate);
}

void bmsoccer_state::grid_w(offs_t offset, u8 data)
{
	// C01: input mux
	if (offset == PORTC)
		m_inp_mux = data & 3;

	// C,D(,E3): vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bmsoccer_state::plate_w(offs_t offset, u8 data)
{
	// G3: speaker out
	if (offset == PORTG)
		m_speaker->level_w(data >> 3 & 1);

	// E012,F012,G012,H,I: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// E3: grid 8
	if (offset == PORTE)
		grid_w(offset, data >> 3 & 1);
	else
		update_display();
}

u8 bmsoccer_state::input_a_r()
{
	// port A: multiplexed inputs
	return read_inputs(2);
}

// config

static INPUT_PORTS_START( bmsoccer )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Ball-carrier Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Ball-carrier Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Ball-carrier Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Ball-carrier Up")

	PORT_START("IN.1") // C1 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("Goalkeeper Left") // note: swap buttons if viewed from the same angle as player 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("Goalkeeper Right")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // port B
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Display/Banana Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Shoot")
INPUT_PORTS_END

void bmsoccer_state::bmsoccer(machine_config &config)
{
	// basic machine hardware
	NEC_D552(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set(FUNC(bmsoccer_state::input_a_r));
	m_maincpu->read_b().set_ioport("IN.2");
	m_maincpu->write_c().set(FUNC(bmsoccer_state::grid_w));
	m_maincpu->write_d().set(FUNC(bmsoccer_state::grid_w));
	m_maincpu->write_e().set(FUNC(bmsoccer_state::plate_w));
	m_maincpu->write_f().set(FUNC(bmsoccer_state::plate_w));
	m_maincpu->write_g().set(FUNC(bmsoccer_state::plate_w));
	m_maincpu->write_h().set(FUNC(bmsoccer_state::plate_w));
	m_maincpu->write_i().set(FUNC(bmsoccer_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(271, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bmsoccer )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-043", 0x0000, 0x0400, CRC(10c2a4ea) SHA1(6ebca7d406e22ff7a8cd529579b55a700da487b4) )

	ROM_REGION( 273796, "screen", 0)
	ROM_LOAD( "bmsoccer.svg", 0, 273796, CRC(4c88d9f8) SHA1(b4b82f26a09f54cd0b6a9d1c1a46796fbfcb578a) )
ROM_END





/***************************************************************************

  Bambino Safari (manufactured in Japan)
  * PCB label Emix Corp. ET-11
  * NEC uCOM-44 MCU, label EMIX D552C 049
  * cyan VFD display Emix-108

***************************************************************************/

class bmsafari_state : public hh_ucom4_state
{
public:
	bmsafari_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void bmsafari(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	void speaker_w(u8 data);
};

// handlers

void bmsafari_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,0,1,2,3,4,5,6,7,8);
	u16 plate = bitswap<16>(m_plate,15,14,13,12,11,7,10,2,9,5,8,4,0,1,6,3);
	m_display->matrix(grid, plate);
}

void bmsafari_state::grid_w(offs_t offset, u8 data)
{
	// C,D(,E3): vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bmsafari_state::plate_w(offs_t offset, u8 data)
{
	// E012,H,I: vfd plate
	int shift = (offset == PORTE) ? 8 : (offset - PORTH) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// E3: grid 0
	if (offset == PORTE)
		grid_w(offset, data >> 3 & 1);
	else
		update_display();
}

void bmsafari_state::speaker_w(u8 data)
{
	// G0: speaker out
	m_speaker->level_w(data & 1);
}

// config

static INPUT_PORTS_START( bmsafari )
	PORT_START("IN.0") // port A
	PORT_CONFNAME( 0x07, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x01, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
INPUT_PORTS_END

void bmsafari_state::bmsafari(machine_config &config)
{
	// basic machine hardware
	NEC_D552(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(bmsafari_state::grid_w));
	m_maincpu->write_d().set(FUNC(bmsafari_state::grid_w));
	m_maincpu->write_e().set(FUNC(bmsafari_state::plate_w));
	m_maincpu->write_g().set(FUNC(bmsafari_state::speaker_w));
	m_maincpu->write_h().set(FUNC(bmsafari_state::plate_w));
	m_maincpu->write_i().set(FUNC(bmsafari_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(248, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 10);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bmsafari )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-049", 0x0000, 0x0400, CRC(82fa3cbe) SHA1(019e7ec784e977eba09997fc46af253054fb222c) )

	ROM_REGION( 275386, "screen", 0)
	ROM_LOAD( "bmsafari.svg", 0, 275386, CRC(c24badbc) SHA1(b191f34155d6d4e834e7c6fe715d4bb76198ad72) )
ROM_END





/***************************************************************************

  Bambino Space Laser Fight (manufactured in Japan)
  * PCB label Emix Corp. ET-12
  * NEC uCOM-43 MCU, label D553C 055
  * cyan VFD display Emix-104, with blue or green color overlay

  This is basically a revamp of their earlier Boxing game (ET-06), case and
  buttons are exactly the same.

***************************************************************************/

class splasfgt_state : public hh_ucom4_state
{
public:
	splasfgt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void splasfgt(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	u8 input_b_r();
};

// handlers

void splasfgt_state::update_display()
{
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,18,17,13,1,0,8,6,0,10,11,14,15,16,9,5,7,4,2,3);
	m_display->matrix(m_grid, plate);
}

void splasfgt_state::grid_w(offs_t offset, u8 data)
{
	// G,H,I0: vfd grid
	int shift = (offset - PORTG) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

	// G(grid 0-3): input mux
	m_inp_mux = m_grid & 0xf;

	// I2: vfd plate 6
	if (offset == PORTI)
		plate_w(4 + PORTC, data >> 2 & 1);
	else
		update_display();
}

void splasfgt_state::plate_w(offs_t offset, u8 data)
{
	// F01: speaker out
	if (offset == PORTF)
		m_speaker->level_w(data & 3);

	// C,D,E,F23(,I2): vfd plate
	int shift = (offset - PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

u8 splasfgt_state::input_b_r()
{
	// B: multiplexed buttons
	return read_inputs(4);
}

// config

/* physical button layout and labels are like this:

    * left = P1 side *                                         * right = P2 side * (note: in 1P mode, switch sides between turns)

    [  JUMP  ]  [ HIGH ]        (players sw)                   [ HIGH ]  [  JUMP  ]
                                1<--->2         [START/
    [STRAIGHT]  [MEDIUM]                         RESET]        [MEDIUM]  [STRAIGHT]
                                1<---OFF--->2
    [ STOOP  ]  [ LOW  ]        (skill lvl sw)                 [ LOW  ]  [ STOOP  ]
*/

static INPUT_PORTS_START( splasfgt )
	PORT_START("IN.0") // G0 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P1 Position Straight")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("P1 Position Jump")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 Position Stoop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // G1 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("P1 Beam High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("P1 Beam Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 Beam Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // G2 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P2 Position Straight")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P2 Position Jump")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P2 Position Stoop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // G3 port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P2 Beam High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P2 Beam Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("P2 Beam Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // port A
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void splasfgt_state::splasfgt(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set_ioport("IN.4");
	m_maincpu->read_b().set(FUNC(splasfgt_state::input_b_r));
	m_maincpu->write_c().set(FUNC(splasfgt_state::plate_w));
	m_maincpu->write_d().set(FUNC(splasfgt_state::plate_w));
	m_maincpu->write_e().set(FUNC(splasfgt_state::plate_w));
	m_maincpu->write_f().set(FUNC(splasfgt_state::plate_w));
	m_maincpu->write_g().set(FUNC(splasfgt_state::grid_w));
	m_maincpu->write_h().set(FUNC(splasfgt_state::grid_w));
	m_maincpu->write_i().set(FUNC(splasfgt_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 476);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	static const double speaker_levels[] = { 0.0, 1.0, -1.0, 0.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( splasfgt )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-055", 0x0000, 0x0800, CRC(eb471fbd) SHA1(f06cfe567bf6f9ed4dcdc88acdcfad50cd370a02) )

	ROM_REGION( 246609, "screen", 0)
	ROM_LOAD( "splasfgt.svg", 0, 246609, CRC(365fae43) SHA1(344c120c2efa92ada9171047affac801a06cf303) )
ROM_END





/***************************************************************************

  Bandai Crazy Climber (manufactured in Japan)
  * PCB labels SM-020/SM-021
  * NEC uCOM-43 MCU, label D553C 170
  * cyan/red/green VFD display NEC FIP6AM2-T no. 1-8 2, with partial color overlay and bezel

  known releases:
  - Japan: FL Crazy Climbing, published by Bandai
  - USA: Crazy Climber, published by Bandai

***************************************************************************/

class bcclimbr_state : public hh_ucom4_state
{
public:
	bcclimbr_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void bcclimbr(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void bcclimbr_state::update_display()
{
	u8 grid = bitswap<8>(m_grid,7,6,0,1,2,3,4,5);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,16,17,18,19,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
	m_display->matrix(grid, plate);
}

void bcclimbr_state::grid_w(offs_t offset, u8 data)
{
	// I2: speaker out
	if (offset == PORTI)
		m_speaker->level_w(data >> 2 & 1);

	// H,I01: vfd grid
	int shift = (offset - PORTH) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bcclimbr_state::plate_w(offs_t offset, u8 data)
{
	// C,D,E,F: vfd plate
	int shift = (offset - PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( bcclimbr )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
INPUT_PORTS_END

void bcclimbr_state::bcclimbr(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(bcclimbr_state::plate_w));
	m_maincpu->write_d().set(FUNC(bcclimbr_state::plate_w));
	m_maincpu->write_e().set(FUNC(bcclimbr_state::plate_w));
	m_maincpu->write_f().set(FUNC(bcclimbr_state::plate_w));
	m_maincpu->write_g().set(FUNC(bcclimbr_state::plate_w));
	m_maincpu->write_h().set(FUNC(bcclimbr_state::grid_w));
	m_maincpu->write_i().set(FUNC(bcclimbr_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(310, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(6, 20);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bcclimbr )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-170", 0x0000, 0x0800, CRC(fc2eabdb) SHA1(0f5cc854be7fdf105d9bd2114659d40c65f9d782) )

	ROM_REGION( 219971, "screen", 0)
	ROM_LOAD( "bcclimbr.svg", 0, 219971, CRC(9c9102f4) SHA1(6a7e02fd1467a26c734b01724e23cef9e4917805) )
ROM_END





/***************************************************************************

  Castle Toy Tactix
  * NEC uCOM-43 MCU, label D557LC 512
  * 16 LEDs behind buttons

  Tactix is similar to Merlin, for 1 or 2 players. In 2-player mode, simply
  don't press the Comp Turn button. The four included minigames are:
  1: Capture (reversi)
  2: Jump-Off (peg solitaire)
  3: Triple Play (3 in a row)
  4: Concentration (memory)

***************************************************************************/

class tactix_state : public hh_ucom4_state
{
public:
	tactix_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void tactix(machine_config &config);

private:
	void leds_w(offs_t offset, u8 data);
	void speaker_w(u8 data);
	void input_w(offs_t offset, u8 data);
	u8 input_r();
};

// handlers

void tactix_state::leds_w(offs_t offset, u8 data)
{
	// D,F: 4*4 led matrix
	m_port[offset] = data;
	m_display->matrix(m_port[PORTF], m_port[PORTD]);
}

void tactix_state::speaker_w(u8 data)
{
	// G0: speaker out
	m_speaker->level_w(data & 1);
}

void tactix_state::input_w(offs_t offset, u8 data)
{
	// C,E0: input mux
	m_port[offset] = data;
	m_inp_mux = (m_port[PORTE] << 4 & 0x10) | m_port[PORTC];
}

u8 tactix_state::input_r()
{
	// A: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( tactix )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Button 9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 13")

	PORT_START("IN.1") // C1 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Button 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Button 10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Button 14")

	PORT_START("IN.2") // C2 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Button 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Button 11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Button 15")

	PORT_START("IN.3") // C3 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Button 8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Button 12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Button 16")

	PORT_START("IN.4") // E0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("New Game")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Comp Turn")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void tactix_state::tactix(machine_config &config)
{
	// basic machine hardware
	NEC_D557L(config, m_maincpu, 200000); // approximation
	m_maincpu->read_a().set(FUNC(tactix_state::input_r));
	m_maincpu->write_c().set(FUNC(tactix_state::input_w));
	m_maincpu->write_d().set(FUNC(tactix_state::leds_w));
	m_maincpu->write_e().set(FUNC(tactix_state::input_w));
	m_maincpu->write_f().set(FUNC(tactix_state::leds_w));
	m_maincpu->write_g().set(FUNC(tactix_state::speaker_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 4);
	config.set_default_layout(layout_tactix);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tactix )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d557lc-512", 0x0000, 0x0800, CRC(1df738cb) SHA1(15a5de28a3c03e6894d29c56b5b424983569ccf2) )
ROM_END





/***************************************************************************

  Castle Toy Name That Tune
  * NEC uCOM-43 MCU, label D557LC 513
  * 2 lamps, 1 7seg(+2 fake 7segs above a power-on lamp, showing "0")

  This is a tabletop multiplayer game. Players are meant to place a bid,
  and guess the song (by announcing it to everyone).

***************************************************************************/

class ctntune_state : public hh_ucom4_state
{
public:
	ctntune_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void ctntune(machine_config &config);

	// start button powers unit back on
	DECLARE_INPUT_CHANGED_MEMBER(start_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE); }

private:
	void update_display();
	void _7seg_w(offs_t offset, u8 data);
	void speaker_w(u8 data);
	void input_w(offs_t offset, u8 data);
	u8 input_r();
};

// handlers

void ctntune_state::update_display()
{
	u8 sel = m_port[PORTD] >> 3 & 1; // turn off display when power is off
	u8 lamps = m_port[PORTD] & 3;
	u8 digit = (m_port[PORTF] << 4 | m_port[PORTE]) & 0x7f;

	m_display->matrix(sel, lamps << 7 | digit);
}

void ctntune_state::_7seg_w(offs_t offset, u8 data)
{
	// E,F012: 7seg data, F3: N/C
	m_port[offset] = data;
	update_display();
}

void ctntune_state::speaker_w(u8 data)
{
	// G0: speaker out
	m_speaker->level_w(data & 1);
}

void ctntune_state::input_w(offs_t offset, u8 data)
{
	// D3: trigger power-off on falling edge
	if (offset == PORTD && ~data & m_port[PORTD] & 8)
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// C,D23: input mux
	// D0,D1: yellow, red lamp
	m_port[offset] = data;
	m_inp_mux = (m_port[PORTD] << 2 & 0x30) | m_port[PORTC];
	update_display();
}

u8 ctntune_state::input_r()
{
	// A: multiplexed inputs
	return read_inputs(6);
}

// config

static INPUT_PORTS_START( ctntune )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Button 1") // defaults to keyboard Z row
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("Button 5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("Button 9")

	PORT_START("IN.1") // C1 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Button 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("Button 10")

	PORT_START("IN.2") // C2 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Button 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Button 7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Yellow Button")

	PORT_START("IN.3") // C3 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Button 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Button 8")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Red Button")

	PORT_START("IN.4") // D2 port A
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Play Button")

	PORT_START("IN.5") // D3 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, ctntune_state, start_button, 0)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void ctntune_state::ctntune(machine_config &config)
{
	// basic machine hardware
	NEC_D557L(config, m_maincpu, 200000); // approximation
	m_maincpu->read_a().set(FUNC(ctntune_state::input_r));
	m_maincpu->write_c().set(FUNC(ctntune_state::input_w));
	m_maincpu->write_d().set(FUNC(ctntune_state::input_w));
	m_maincpu->write_e().set(FUNC(ctntune_state::_7seg_w));
	m_maincpu->write_f().set(FUNC(ctntune_state::_7seg_w));
	m_maincpu->write_g().set(FUNC(ctntune_state::speaker_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 7+2);
	m_display->set_segmask(1, 0x7f);
	config.set_default_layout(layout_ctntune);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ctntune )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d557lc-513", 0x0000, 0x0800, CRC(cd85ee23) SHA1(32b8fc8cb92fc1fd27da9148788a09d3bcd46a92) )
ROM_END





/***************************************************************************

  Epoch Invader From Space (manufactured in Japan)
  * PCB labels 36010(A/B)
  * NEC uCOM-44 MCU, label D552C 054
  * cyan VFD display NEC FIP9AM18T tube no. 0D, with color overlay

  known releases:
  - USA: Invader From Space, published by Epoch
  - UK: Invader From Space, published by Grandstand

***************************************************************************/

class invspace_state : public hh_ucom4_state
{
public:
	invspace_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void invspace(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void invspace_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,8,9,7,6,5,4,3,2,1,0);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,9,14,13,8,15,11,10,7,11,3,2,6,10,1,5,9,0,4,8);
	m_display->matrix(grid, plate);
}

void invspace_state::grid_w(offs_t offset, u8 data)
{
	// I0: speaker out
	if (offset == PORTI)
		m_speaker->level_w(data & 1);

	// C,D,I1: vfd grid
	int shift = (offset == PORTI) ? 8 : (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void invspace_state::plate_w(offs_t offset, u8 data)
{
	// E,F,G,H123: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( invspace )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void invspace_state::invspace(machine_config &config)
{
	// basic machine hardware
	NEC_D552(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(invspace_state::grid_w));
	m_maincpu->write_d().set(FUNC(invspace_state::grid_w));
	m_maincpu->write_e().set(FUNC(invspace_state::plate_w));
	m_maincpu->write_f().set(FUNC(invspace_state::plate_w));
	m_maincpu->write_g().set(FUNC(invspace_state::plate_w));
	m_maincpu->write_h().set(FUNC(invspace_state::plate_w));
	m_maincpu->write_i().set(FUNC(invspace_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(289, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 19);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( invspace )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-054", 0x0000, 0x0400, CRC(913d9c13) SHA1(f20edb5458e54d2f6d4e45e5d59efd87e05a6f3f) )

	ROM_REGION( 110899, "screen", 0)
	ROM_LOAD( "invspace.svg", 0, 110899, CRC(ae794333) SHA1(3552215389f02e4ef1d608f7dfc84f0499a78ee2) )
ROM_END





/***************************************************************************

  Epoch Electronic Football (manufactured in Japan)
  * PCB labels 36020(A/B/C)
  * NEC uCOM-43 MCU, label D553C 080
  * cyan VFD display NEC FIP10AM15T tube no. 0F, with bezel overlay

  known releases:
  - USA: Electronic Football (aka Pro-Bowl Football), published by Epoch
  - Japan: American Football, published by Epoch

***************************************************************************/

class efball_state : public hh_ucom4_state
{
public:
	efball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void efball(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void efball_state::update_display()
{
	u16 plate = bitswap<16>(m_plate,15,14,13,12,11,4,3,0,2,1,6,10,9,5,8,7);
	m_display->matrix(m_grid, plate);
}

void efball_state::grid_w(offs_t offset, u8 data)
{
	// H2: speaker out
	if (offset == PORTH)
		m_speaker->level_w(data >> 2 & 1);

	// F,G,H01: vfd grid
	int shift = (offset - PORTF) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void efball_state::plate_w(offs_t offset, u8 data)
{
	// D,E,I: vfd plate
	int shift = (offset == PORTI) ? 8 : (offset - PORTD) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( efball )
	PORT_START("IN.0") // port A
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Down-Field")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P1 Score-Time")

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, NOTEQUALS, 0x00) // left/right
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass")

	PORT_START("IN.2") // port C
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Kick Return")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Kick")

	PORT_START("FAKE") // fake port for left/right combination
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Left/Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Left/Right")
INPUT_PORTS_END

void efball_state::efball(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->read_c().set_ioport("IN.2");
	m_maincpu->write_d().set(FUNC(efball_state::plate_w));
	m_maincpu->write_e().set(FUNC(efball_state::plate_w));
	m_maincpu->write_f().set(FUNC(efball_state::grid_w));
	m_maincpu->write_g().set(FUNC(efball_state::grid_w));
	m_maincpu->write_h().set(FUNC(efball_state::grid_w));
	m_maincpu->write_i().set(FUNC(efball_state::plate_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 11);
	config.set_default_layout(layout_efball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( efball )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-080", 0x0000, 0x0800, CRC(54c1027f) SHA1(6cc98074dae9361fa8c0ed6501b6a57ad325ccbd) )
ROM_END





/***************************************************************************

  Epoch Galaxy II (manufactured in Japan)
  * PCB labels 19096/96062
  * NEC uCOM-43 MCU, label D553C 153
  * cyan/red VFD display NEC FIP10xM20T, with color overlay. x = multiple VFD
    revisions exist, with different graphics: rev B no. 1-8, rev. D no. 2-21.

  known releases:
  - USA: Galaxy II, published by Epoch
  - Japan: Astro Wars, published by Epoch
  - UK: Astro Wars, published by Grandstand

***************************************************************************/

class galaxy2_state : public hh_ucom4_state
{
public:
	galaxy2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void galaxy2b(machine_config &config);
	void galaxy2(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void galaxy2_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	u16 plate = bitswap<16>(m_plate,15,3,2,6,1,5,4,0,11,10,7,12,14,13,8,9);
	m_display->matrix(grid, plate);
}

void galaxy2_state::grid_w(offs_t offset, u8 data)
{
	// E3: speaker out
	if (offset == PORTE)
		m_speaker->level_w(data >> 3 & 1);

	// C,D,E01: vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void galaxy2_state::plate_w(offs_t offset, u8 data)
{
	// F,G,H,I: vfd plate
	int shift = (offset - PORTF) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( galaxy2 )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void galaxy2_state::galaxy2(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(galaxy2_state::grid_w));
	m_maincpu->write_d().set(FUNC(galaxy2_state::grid_w));
	m_maincpu->write_e().set(FUNC(galaxy2_state::grid_w));
	m_maincpu->write_f().set(FUNC(galaxy2_state::plate_w));
	m_maincpu->write_g().set(FUNC(galaxy2_state::plate_w));
	m_maincpu->write_h().set(FUNC(galaxy2_state::plate_w));
	m_maincpu->write_i().set(FUNC(galaxy2_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(304, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 15);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void galaxy2_state::galaxy2b(machine_config &config)
{
	galaxy2(config);

	// video hardware
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(306, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( galaxy2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-153.s01", 0x0000, 0x0800, CRC(70d552b3) SHA1(72d50647701cb4bf85ea947a149a317aaec0f52c) )

	ROM_REGION( 325057, "screen", 0)
	ROM_LOAD( "galaxy2d.svg", 0, 325057, CRC(b2d27a0e) SHA1(502ec22c324903ffe8ff235b9a3b8898dce17a64) )
ROM_END

ROM_START( galaxy2b )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-153.s01", 0x0000, 0x0800, CRC(70d552b3) SHA1(72d50647701cb4bf85ea947a149a317aaec0f52c) )

	ROM_REGION( 266377, "screen", 0)
	ROM_LOAD( "galaxy2b.svg", 0, 266377, CRC(8633cebb) SHA1(6c41f5e918e1522eb55ef24270900a1b2477722b) )
ROM_END





/***************************************************************************

  Epoch Astro Command (manufactured in Japan)
  * PCB labels 96111/96112
  * NEC uCOM-43 MCU, label D553C 202
  * cyan/red VFD display NEC FIP9AM20T no. 42-42, with color overlay + bezel

  known releases:
  - Japan: Astro Command, published by Epoch
  - USA: Astro Command, published by Tandy
  - UK: Scramble, published by Grandstand

***************************************************************************/

class astrocmd_state : public hh_ucom4_state
{
public:
	astrocmd_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void astrocmd(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void astrocmd_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,8,4,5,6,7,0,1,2,3);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,3,2,12,13,14,15,16,17,18,0,1,4,8,5,9,7,11,6,10);
	m_display->matrix(grid, plate);
}

void astrocmd_state::grid_w(offs_t offset, u8 data)
{
	// C,D(,E3): vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void astrocmd_state::plate_w(offs_t offset, u8 data)
{
	// E01,F,G,H,I: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	if (offset == PORTE)
	{
		// E2: speaker out
		m_speaker->level_w(data >> 2 & 1);

		// E3: vfd grid 8
		grid_w(offset, data >> 3 & 1);
	}
	else
		update_display();
}

// config

static INPUT_PORTS_START( astrocmd )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Missile")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bomb")

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

void astrocmd_state::astrocmd(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(astrocmd_state::grid_w));
	m_maincpu->write_d().set(FUNC(astrocmd_state::grid_w));
	m_maincpu->write_e().set(FUNC(astrocmd_state::plate_w));
	m_maincpu->write_f().set(FUNC(astrocmd_state::plate_w));
	m_maincpu->write_g().set(FUNC(astrocmd_state::plate_w));
	m_maincpu->write_h().set(FUNC(astrocmd_state::plate_w));
	m_maincpu->write_i().set(FUNC(astrocmd_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 525);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 17);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( astrocmd )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-202.s01", 0x0000, 0x0800, CRC(b4b34883) SHA1(6246d561c2df1f2124575d2ca671ef85b1819edd) )

	ROM_REGION( 335362, "screen", 0)
	ROM_LOAD( "astrocmd.svg", 0, 335362, CRC(fe2cd30f) SHA1(898a3d9afc5dca6c63ae28aed2c8530716ad1c45) )
ROM_END





/***************************************************************************

  Epoch Dracula (manufactured in Japan)
  * PCB label 96121
  * NEC uCOM-43 MCU, label D553C 206
  * cyan/red/green VFD display NEC FIP8BM20T no. 2-42

  known releases:
  - Japan: Dracula House, yellow case, published by Epoch
  - USA: Dracula, red case, published by Epoch
  - Other: Dracula, yellow case, published by Hales

***************************************************************************/

class edracula_state : public hh_ucom4_state
{
public:
	edracula_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void edracula(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void edracula_state::update_display()
{
	m_display->matrix(m_grid, m_plate);
}

void edracula_state::grid_w(offs_t offset, u8 data)
{
	// C,D: vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void edracula_state::plate_w(offs_t offset, u8 data)
{
	// I2: speaker out
	if (offset == PORTI)
		m_speaker->level_w(data >> 2 & 1);

	// E,F,G,H,I01: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( edracula )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

void edracula_state::edracula(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(edracula_state::grid_w));
	m_maincpu->write_d().set(FUNC(edracula_state::grid_w));
	m_maincpu->write_e().set(FUNC(edracula_state::plate_w));
	m_maincpu->write_f().set(FUNC(edracula_state::plate_w));
	m_maincpu->write_g().set(FUNC(edracula_state::plate_w));
	m_maincpu->write_h().set(FUNC(edracula_state::plate_w));
	m_maincpu->write_i().set(FUNC(edracula_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 526);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 18);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( edracula )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-206.s01", 0x0000, 0x0800, CRC(b524857b) SHA1(c1c89ed5dd4bb1e6e98462dc8fa5af2aa48d8ede) )

	ROM_REGION( 794532, "screen", 0)
	ROM_LOAD( "edracula.svg", 0, 794532, CRC(d20e018c) SHA1(7f70f1d373c034ec8c93e27b7e3371578ddaf61b) )
ROM_END





/***************************************************************************

  Mattel Computer Gin
  * NEC uCOM-43 MCU, label D650C 060 (die label same)
  * Hughes HLCD0530 LCD driver, 5 by 14 segments LCD panel, no sound

***************************************************************************/

class mcompgin_state : public hh_ucom4_state
{
public:
	mcompgin_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag),
		m_lcd(*this, "lcd")
	{ }

	void mcompgin(machine_config &config);

private:
	required_device<hlcd0530_device> m_lcd;

	void lcd_output_w(offs_t offset, u32 data);
	void lcd_w(u8 data);
};

// handlers

void mcompgin_state::lcd_output_w(offs_t offset, u32 data)
{
	// uses ROW0-4, COL11-24
	m_display->matrix(1 << offset, data);
}

void mcompgin_state::lcd_w(u8 data)
{
	// E0: HLCD0530 _CS
	// E1: HLCD0530 clock
	// E2: HLCD0530 data in
	m_lcd->cs_w(data & 1);
	m_lcd->data_w(data >> 2 & 1);
	m_lcd->clock_w(data >> 1 & 1);
}

// config

static INPUT_PORTS_START( mcompgin )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Deal / Gin")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Discard")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Draw")

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Compare")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Score")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void mcompgin_state::mcompgin(machine_config &config)
{
	// basic machine hardware
	NEC_D650(config, m_maincpu, 400_kHz_XTAL); // TDK FCR400K
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_e().set(FUNC(mcompgin_state::lcd_w));

	// video hardware
	HLCD0530(config, m_lcd, 500); // C=0.01uF
	m_lcd->write_cols().set(FUNC(mcompgin_state::lcd_output_w));

	PWM_DISPLAY(config, m_display).set_size(8, 24);

	config.set_default_layout(layout_mcompgin);

	// no sound!
}

// roms

ROM_START( mcompgin )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d650c-060", 0x0000, 0x0800, CRC(985e6da6) SHA1(ea4102a10a5741f06297c5426156e4b2f0d85a68) )
ROM_END





/***************************************************************************

  Mego Mini-Vid: Break Free (manufactured in Japan)
  * PCB label Mego 79 rev F
  * NEC uCOM-43 MCU, label D553C 049
  * cyan VFD display Futaba DM-4.5 91

***************************************************************************/

class mvbfree_state : public hh_ucom4_state
{
public:
	mvbfree_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void mvbfree(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	void speaker_w(u8 data);
};

// handlers

void mvbfree_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
	u16 plate = bitswap<16>(m_plate,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	m_display->matrix(grid, plate);
}

void mvbfree_state::grid_w(offs_t offset, u8 data)
{
	// E23,F,G,H: vfd grid
	int shift = (offset - PORTE) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

	// E01: plate 0,1
	if (offset == PORTE)
		plate_w(2 + PORTC, data & 3);
	else
		update_display();
}

void mvbfree_state::plate_w(offs_t offset, u8 data)
{
	// C,D(,E01): vfd plate
	int shift = (offset - PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void mvbfree_state::speaker_w(u8 data)
{
	// I0: speaker out
	m_speaker->level_w(data & 1);
}

// config

static INPUT_PORTS_START( mvbfree )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.1") // port B
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED ) // unimplemented p1/p2 buttons
	PORT_CONFNAME( 0x0c, 0x04, "Game Select")
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFSETTING(    0x08, "3" )
INPUT_PORTS_END

void mvbfree_state::mvbfree(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(mvbfree_state::plate_w));
	m_maincpu->write_d().set(FUNC(mvbfree_state::plate_w));
	m_maincpu->write_e().set(FUNC(mvbfree_state::grid_w));
	m_maincpu->write_f().set(FUNC(mvbfree_state::grid_w));
	m_maincpu->write_g().set(FUNC(mvbfree_state::grid_w));
	m_maincpu->write_h().set(FUNC(mvbfree_state::grid_w));
	m_maincpu->write_i().set(FUNC(mvbfree_state::speaker_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(14, 10);
	config.set_default_layout(layout_mvbfree);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mvbfree )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-049", 0x0000, 0x0800, CRC(d64a8399) SHA1(97887e486fa29b1fc4a5a40cacf3c960f67aacbf) )
ROM_END





/***************************************************************************

  Takatoku Toys(T.T) Game Robot 9 「ゲームロボット九」
  * PCB label GAME ROBOT 7520
  * NEC uCOM-43 MCU, label TTGR-512 (die label NEC D557 511)
  * 9 lamps behind buttons

  known releases:
  - Japan: Game Robot 9, published by Takatoku Toys
  - USA: Fabulous Fred - The Ultimate Electronic Game, published by Mego
  - Mexico: Fabuloso Fred, published by Ensueño Toys (also released as
    12-button version, a clone of Tandy-12)

  Accessories were included for some of the minigames.

***************************************************************************/

class grobot9_state : public hh_ucom4_state
{
public:
	grobot9_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void grobot9(machine_config &config);

private:
	void lamps_w(offs_t offset, u8 data);
	void speaker_w(u8 data);
	void input_w(u8 data);
	u8 input_r();
};

// handlers

void grobot9_state::lamps_w(offs_t offset, u8 data)
{
	if (offset == PORTE)
	{
		// E1: speaker out
		m_speaker->level_w(data >> 1 & 1);

		// E3: input mux high bit
		m_inp_mux = (m_inp_mux & 7) | (data & 8);
	}

	// D,F,E0: lamps
	m_port[offset] = data;
	m_display->matrix(1, m_port[PORTD] | m_port[PORTF] << 4 | m_port[PORTE] << 8);
}

void grobot9_state::input_w(u8 data)
{
	// C012: input mux low
	m_inp_mux = (m_inp_mux & 8) | (data & 7);
}

u8 grobot9_state::input_r()
{
	// A: multiplexed inputs
	return read_inputs(5);
}

// config

static INPUT_PORTS_START( grobot9 )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_W) PORT_NAME("Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_E) PORT_NAME("Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_D) PORT_NAME("Button 4")

	PORT_START("IN.1") // C1 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_C) PORT_NAME("Button 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_X) PORT_NAME("Button 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_A) PORT_NAME("Button 8")

	PORT_START("IN.2") // C2 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_S) PORT_NAME("Button 9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Rest")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Eighth Note")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // E3 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Hit")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Repeat")

	PORT_START("IN.4") // INT
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_ucom4_state, single_interrupt_line, 0) PORT_NAME("Start-Pitch")
INPUT_PORTS_END

void grobot9_state::grobot9(machine_config &config)
{
	// basic machine hardware
	NEC_D557L(config, m_maincpu, 160000); // approximation
	m_maincpu->read_a().set(FUNC(grobot9_state::input_r));
	m_maincpu->write_c().set(FUNC(grobot9_state::input_w));
	m_maincpu->write_d().set(FUNC(grobot9_state::lamps_w));
	m_maincpu->write_e().set(FUNC(grobot9_state::lamps_w));
	m_maincpu->write_f().set(FUNC(grobot9_state::lamps_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 9);
	config.set_default_layout(layout_grobot9);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( grobot9 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "ttgr-511", 0x0000, 0x0800, CRC(1f25b2bb) SHA1(55ae7e23f6dd46cc6e1a65839327726678410c3a) )
ROM_END





/***************************************************************************

  Tomy(tronic) Cosmic Combat (manufactured in Japan)
  * PCB label 2E1019-E01
  * NEC uCOM-44 MCU, label D552C 042
  * cyan VFD display NEC FIP32AM18Y tube no. 0E, with color overlay

  known releases:
  - USA: Cosmic Combat, published by Tomy
  - Japan: Space Attack, published by Tomy

***************************************************************************/

class tccombat_state : public hh_ucom4_state
{
public:
	tccombat_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void tccombat(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void tccombat_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,8,3,2,1,0,7,6,5,4);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,11,15,3,10,14,2,9,13,1,0,12,8,15,1,5,0,3,7,2,6);
	m_display->matrix(grid, plate);
}

void tccombat_state::grid_w(offs_t offset, u8 data)
{
	// I1: speaker out
	if (offset == PORTI)
		m_speaker->level_w(data >> 1 & 1);

	// C,D,I0: vfd grid
	int shift = (offset == PORTI) ? 8 : (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tccombat_state::plate_w(offs_t offset, u8 data)
{
	// E,F123,G,H: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( tccombat )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
INPUT_PORTS_END

void tccombat_state::tccombat(machine_config &config)
{
	// basic machine hardware
	NEC_D552(config, m_maincpu, 400000); // approximation
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_c().set(FUNC(tccombat_state::grid_w));
	m_maincpu->write_d().set(FUNC(tccombat_state::grid_w));
	m_maincpu->write_e().set(FUNC(tccombat_state::plate_w));
	m_maincpu->write_f().set(FUNC(tccombat_state::plate_w));
	m_maincpu->write_g().set(FUNC(tccombat_state::plate_w));
	m_maincpu->write_h().set(FUNC(tccombat_state::plate_w));
	m_maincpu->write_i().set(FUNC(tccombat_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(300, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 20);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tccombat )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-042", 0x0000, 0x0400, CRC(d7b5cfeb) SHA1(a267be8e43b7740758eb0881b655b1cc8aec43da) )

	ROM_REGION( 210960, "screen", 0)
	ROM_LOAD( "tccombat.svg", 0, 210960, CRC(03e9eba6) SHA1(d558d3063da42dc7cc02b769bca06a3732418837) )
ROM_END





/***************************************************************************

  Tomy(tronic) Tennis (manufactured in Japan)
  * PCB label TOMY TN-04 TENNIS
  * NEC uCOM-44 MCU, label D552C 048
  * cyan VFD display NEC FIP11AM15T tube no. 0F, with overlay

  The initial release of this game was in 1979, known as Pro-Tennis,
  it has a D553 instead of D552, with just a little over 50% ROM used.

  Press the Serve button to start, then hit the ball by pressing one of the
  positional buttons when the ball flies over it.

***************************************************************************/

class tmtennis_state : public hh_ucom4_state
{
public:
	tmtennis_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void tmtennis(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(difficulty_switch) { set_clock(); }

protected:
	virtual void machine_reset() override;

private:
	void set_clock();
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
	void port_e_w(u8 data);
	u8 input_r(offs_t offset);
};

void tmtennis_state::machine_reset()
{
	hh_ucom4_state::machine_reset();
	set_clock();
}

// handlers

void tmtennis_state::set_clock()
{
	// MCU clock is from an LC circuit oscillating by default at ~360kHz,
	// but on PRO1, the difficulty switch puts a capacitor across the LC circuit
	// to slow it down to ~260kHz.
	m_maincpu->set_unscaled_clock((m_inputs[1]->read() & 0x100) ? 260000 : 360000);
}

void tmtennis_state::update_display()
{
	m_display->matrix(m_grid, m_plate);
}

void tmtennis_state::grid_w(offs_t offset, u8 data)
{
	// G,H,I: vfd grid
	int shift = (offset - PORTG) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tmtennis_state::plate_w(offs_t offset, u8 data)
{
	// C,D,F: vfd plate
	int shift = (offset == PORTF) ? 8 : (offset - PORTC) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tmtennis_state::port_e_w(u8 data)
{
	// E01: input mux
	// E2: speaker out
	// E3: N/C
	m_inp_mux = data & 3;
	m_speaker->level_w(data >> 2 & 1);
}

u8 tmtennis_state::input_r(offs_t offset)
{
	// A,B: multiplexed buttons
	return ~read_inputs(2) >> (offset*4);
}

// config

/* Pro-Tennis physical button layout and labels are like this:

    * left = P2/CPU side *    * right = P1 side *

    [SERVE] [1] [2] [3]       [3] [2] [1] [SERVE]
            [4] [5] [6]       [6] [5] [4]

    PRACTICE<--PRO1-->PRO2    1PLAYER<--OFF-->2PLAYER
*/

static INPUT_PORTS_START( tmtennis )
	PORT_START("IN.0") // E0 port A/B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("P1 Serve")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("P2 Serve")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("P1 Button 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("P1 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 Button 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P1 Button 5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P1 Button 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 Button 6")

	PORT_START("IN.1") // E1 port A/B
	PORT_CONFNAME( 0x101, 0x100, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, tmtennis_state, difficulty_switch, 0)
	PORT_CONFSETTING(     0x001, "Practice" )
	PORT_CONFSETTING(     0x100, "Pro 1" ) // -> difficulty_switch
	PORT_CONFSETTING(     0x000, "Pro 2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("P2 Button 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("P2 Button 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 Button 5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("P2 Button 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 Button 6")
INPUT_PORTS_END

void tmtennis_state::tmtennis(machine_config &config)
{
	// basic machine hardware
	NEC_D552(config, m_maincpu, 360000); // see set_clock
	m_maincpu->read_a().set(FUNC(tmtennis_state::input_r));
	m_maincpu->read_b().set(FUNC(tmtennis_state::input_r));
	m_maincpu->write_c().set(FUNC(tmtennis_state::plate_w));
	m_maincpu->write_d().set(FUNC(tmtennis_state::plate_w));
	m_maincpu->write_e().set(FUNC(tmtennis_state::port_e_w));
	m_maincpu->write_f().set(FUNC(tmtennis_state::plate_w));
	m_maincpu->write_g().set(FUNC(tmtennis_state::grid_w));
	m_maincpu->write_h().set(FUNC(tmtennis_state::grid_w));
	m_maincpu->write_i().set(FUNC(tmtennis_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 417);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(12, 12);
	config.set_default_layout(layout_tmtennis);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmtennis )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "d552c-048", 0x0000, 0x0400, CRC(78702003) SHA1(4d427d4dbeed901770c682338867f58c7b54eee3) )

	ROM_REGION( 204490, "screen", 0)
	ROM_LOAD( "tmtennis.svg", 0, 204490, CRC(ed0086e9) SHA1(26a5b2f0a9cd70401187146e1495aee80020658b) )
ROM_END





/***************************************************************************

  Tomy(tronic) Pac-Man (manufactured in Japan)
  * PCB label TN-08 2E108E01
  * NEC uCOM-43 MCU, label D553C 160
  * cyan/red/green VFD display NEC FIP8AM18T no. 2-21
  * bright yellow round casing

  known releases:
  - Japan: Puck Man, published by Tomy
  - USA: Pac Man, published by Tomy
  - UK: Puckman (Tomy), and also published by Grandstand as Munchman
  - Australia: Pac Man-1, published by Futuretronics

  The game will start automatically after turning it on. This Pac Man refuses
  to eat dots with his butt, you can only eat them going right-to-left.

***************************************************************************/

class tmpacman_state : public hh_ucom4_state
{
public:
	tmpacman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void tmpacman(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void tmpacman_state::update_display()
{
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,6,7);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,16,17,18,11,10,9,8,0,2,3,1,4,5,6,7,12,13,14,15) | 0x100;
	m_display->matrix(grid, plate);
}

void tmpacman_state::grid_w(offs_t offset, u8 data)
{
	// C,D: vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tmpacman_state::plate_w(offs_t offset, u8 data)
{
	// E1: speaker out
	if (offset == PORTE)
		m_speaker->level_w(data >> 1 & 1);

	// E023,F,G,H,I: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( tmpacman )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // port B
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void tmpacman_state::tmpacman(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 430_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(tmpacman_state::grid_w));
	m_maincpu->write_d().set(FUNC(tmpacman_state::grid_w));
	m_maincpu->write_e().set(FUNC(tmpacman_state::plate_w));
	m_maincpu->write_f().set(FUNC(tmpacman_state::plate_w));
	m_maincpu->write_g().set(FUNC(tmpacman_state::plate_w));
	m_maincpu->write_h().set(FUNC(tmpacman_state::plate_w));
	m_maincpu->write_i().set(FUNC(tmpacman_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 508);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 19);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmpacman )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-160", 0x0000, 0x0800, CRC(b21a8af7) SHA1(e3122be1873ce76a4067386bf250802776f0c2f9) )

	ROM_REGION( 230216, "screen", 0)
	ROM_LOAD( "tmpacman.svg", 0, 230216, CRC(2ab5c0f1) SHA1(b2b6482b03c28515dc76fd3d6034c8b7e6bf6efc) )
ROM_END





/***************************************************************************

  Tomy(tronic) Scramble (manufactured in Japan)
  * PCB label TN-10 2E114E01
  * NEC uCOM-43 MCU, label D553C 192
  * cyan/red/green VFD display NEC FIP10CM20T no. 2-41

  known releases:
  - World: Scramble, published by Tomy
  - USA: Scramble, published by Tandy
  - UK: Astro Blaster, published by Hales (Epoch Astro Command was named Scramble)
  - Germany: Rambler, published by Tomy

***************************************************************************/

class tmscramb_state : public hh_ucom4_state
{
public:
	tmscramb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void tmscramb(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void tmscramb_state::update_display()
{
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,18,17,3,15,2,14,1,13,16,0,12,8,4,9,5,10,6,11,7) | 0x400;
	m_display->matrix(m_grid, plate);
}

void tmscramb_state::grid_w(offs_t offset, u8 data)
{
	// I2: speaker out
	if (offset == PORTI)
		m_speaker->level_w(data >> 2 & 1);

	// C,D,I01: vfd grid
	int shift = (offset == PORTI) ? 8 : (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tmscramb_state::plate_w(offs_t offset, u8 data)
{
	// E,F,G,H: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( tmscramb )
	PORT_START("IN.0") // port A
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x01, "Professional" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1") // port B
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void tmscramb_state::tmscramb(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->read_b().set_ioport("IN.1");
	m_maincpu->write_c().set(FUNC(tmscramb_state::grid_w));
	m_maincpu->write_d().set(FUNC(tmscramb_state::grid_w));
	m_maincpu->write_e().set(FUNC(tmscramb_state::plate_w));
	m_maincpu->write_f().set(FUNC(tmscramb_state::plate_w));
	m_maincpu->write_g().set(FUNC(tmscramb_state::plate_w));
	m_maincpu->write_h().set(FUNC(tmscramb_state::plate_w));
	m_maincpu->write_i().set(FUNC(tmscramb_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 556);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 17);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmscramb )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-192", 0x0000, 0x0800, CRC(00fcc501) SHA1(a7771e934bf8268c83f38c7ec0acc668836e0939) )

	ROM_REGION( 243830, "screen", 0)
	ROM_LOAD( "tmscramb.svg", 0, 243830, CRC(300b098a) SHA1(9fde58ac0f4e4cfea05301346cbf5b1ced9fe973) )
ROM_END





/***************************************************************************

  Tomy(tronic) Caveman (manufactured in Japan)
  * PCB label TN-12 2E114E03
  * NEC uCOM-43 MCU, label D553C 209
  * cyan/red/green VFD display NEC FIP8AM20T no. 2-42

  known releases:
  - World: Caveman, published by Tomy
  - USA: Caveman, published by Tandy
  - UK: Cave Man - Jr. Caveman vs Dinosaur, published by Grandstand

***************************************************************************/

class tcaveman_state : public hh_ucom4_state
{
public:
	tcaveman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void tcaveman(machine_config &config);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(offs_t offset, u8 data);
};

// handlers

void tcaveman_state::update_display()
{
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,6,7);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,10,11,5,6,7,8,0,9,2,18,17,16,3,15,14,13,12,4,1) | 0x40;
	m_display->matrix(grid, plate);
}

void tcaveman_state::grid_w(offs_t offset, u8 data)
{
	// C,D: vfd grid
	int shift = (offset - PORTC) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tcaveman_state::plate_w(offs_t offset, u8 data)
{
	// E3: speaker out
	if (offset == PORTE)
		m_speaker->level_w(data >> 3 & 1);

	// E012,F,G,H,I: vfd plate
	int shift = (offset - PORTE) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

// config

static INPUT_PORTS_START( tcaveman )
	PORT_START("IN.0") // port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x08, "Professional" )
INPUT_PORTS_END

void tcaveman_state::tcaveman(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set_ioport("IN.0");
	m_maincpu->write_c().set(FUNC(tcaveman_state::grid_w));
	m_maincpu->write_d().set(FUNC(tcaveman_state::grid_w));
	m_maincpu->write_e().set(FUNC(tcaveman_state::plate_w));
	m_maincpu->write_f().set(FUNC(tcaveman_state::plate_w));
	m_maincpu->write_g().set(FUNC(tcaveman_state::plate_w));
	m_maincpu->write_h().set(FUNC(tcaveman_state::plate_w));
	m_maincpu->write_i().set(FUNC(tcaveman_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 559);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 19);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tcaveman )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-209", 0x0000, 0x0800, CRC(d230d4b7) SHA1(2fb12b60410f5567c5e3afab7b8f5aa855d283be) )

	ROM_REGION( 306952, "screen", 0)
	ROM_LOAD( "tcaveman.svg", 0, 306952, CRC(a0588b14) SHA1(f67edf579963fc19bc7f9d268329cbc0230712d8) )
ROM_END





/***************************************************************************

  Tomy Alien Chase (manufactured in Japan)
  * PCB label TN-16 2E121B01
  * NEC uCOM-43 MCU, label D553C 258
  * red/green VFD display NEC FIP9AM24T, with color overlay, 2-sided*

  *Player one views the VFD from the front (grid+filament side) while the
  opposite player views it from the back side (through the conductive traces),
  basically a mirror-image.

  To start the game, simply press [UP]. Hold a joystick direction to move around.

***************************************************************************/

class alnchase_state : public hh_ucom4_state
{
public:
	alnchase_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_ucom4_state(mconfig, type, tag)
	{ }

	void alnchase(machine_config &config);

private:
	void output_w(offs_t offset, u8 data);
	u8 input_r();
};

// handlers

void alnchase_state::output_w(offs_t offset, u8 data)
{
	if (offset <= PORTE)
	{
		// C,D,E0: vfd grid
		int shift = (offset - PORTC) * 4;
		m_grid = (m_grid & ~(0xf << shift)) | (data << shift);

		// C0(grid 0): input enable PL1
		// D0(grid 4): input enable PL2
		m_inp_mux = (m_grid & 1) | (m_grid >> 3 & 2);

		// E1: speaker out
		if (offset == PORTE)
			m_speaker->level_w(data >> 1 & 1);
	}

	if (offset >= PORTE)
	{
		// E23,F,G,H,I: vfd plate
		int shift = (offset - PORTE) * 4;
		m_plate = ((m_plate << 2 & ~(0xf << shift)) | (data << shift)) >> 2;
	}

	m_display->matrix(m_grid, m_plate);
}

u8 alnchase_state::input_r()
{
	// A: multiplexed buttons
	return read_inputs(2);
}

// config

/* physical button layout and labels are like this:

    POWER SOUND LEVEL PLAYER
     ON    ON    PRO   TWO        START
      o     o     |     |
      |     |     |     |       [joystick]
      |     |     o     o
     OFF   OFF   AMA   ONE     GAME 0,1,2,3

    1 PLAYER SIDE

    other player side only has a joystick
*/

static INPUT_PORTS_START( alnchase )
	PORT_START("IN.0") // C0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.1") // D0 port A
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) // on non-mirrored view, swap P2 left/right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)

	PORT_START("IN.2") // port B
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x02, "Professional" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void alnchase_state::alnchase(machine_config &config)
{
	// basic machine hardware
	NEC_D553(config, m_maincpu, 400_kHz_XTAL);
	m_maincpu->read_a().set(FUNC(alnchase_state::input_r));
	m_maincpu->read_b().set_ioport("IN.2");
	m_maincpu->write_c().set(FUNC(alnchase_state::output_w));
	m_maincpu->write_d().set(FUNC(alnchase_state::output_w));
	m_maincpu->write_e().set(FUNC(alnchase_state::output_w));
	m_maincpu->write_f().set(FUNC(alnchase_state::output_w));
	m_maincpu->write_g().set(FUNC(alnchase_state::output_w));
	m_maincpu->write_h().set(FUNC(alnchase_state::output_w));
	m_maincpu->write_i().set(FUNC(alnchase_state::output_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(365, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 17);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( alnchase )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "d553c-258", 0x0000, 0x0800, CRC(c5284ff5) SHA1(6a20aaacc9748f0e0335958f3cea482e36153704) )

	ROM_REGION( 576864, "screen", 0)
	ROM_LOAD( "alnchase.svg", 0, 576864, CRC(fe7c7078) SHA1(0d201eeaeb291ded14c0759d1d3d5b2491cf0792) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT   CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, ufombs,   0,        0, ufombs,   ufombs,   ufombs_state,   empty_init, "Bambino", "UFO Master-Blaster Station", MACHINE_SUPPORTS_SAVE )
CONS( 1979, ssfball,  0,        0, ssfball,  ssfball,  ssfball_state,  empty_init, "Bambino", "Superstar Football (Bambino)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, bmcfball, ssfball,  0, ssfball,  ssfball,  ssfball_state,  empty_init, "Bambino", "Classic Football (Bambino)", MACHINE_SUPPORTS_SAVE )
CONS( 1979, bmsoccer, 0,        0, bmsoccer, bmsoccer, bmsoccer_state, empty_init, "Bambino", "Kick The Goal Soccer", MACHINE_SUPPORTS_SAVE )
CONS( 1981, bmsafari, 0,        0, bmsafari, bmsafari, bmsafari_state, empty_init, "Bambino", "Safari (Bambino)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, splasfgt, 0,        0, splasfgt, splasfgt, splasfgt_state, empty_init, "Bambino", "Space Laser Fight", MACHINE_SUPPORTS_SAVE )

CONS( 1982, bcclimbr, 0,        0, bcclimbr, bcclimbr, bcclimbr_state, empty_init, "Bandai", "Crazy Climber (Bandai)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, tactix,   0,        0, tactix,   tactix,   tactix_state,   empty_init, "Castle Toy", "Tactix (Castle Toy)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, ctntune,  0,        0, ctntune,  ctntune,  ctntune_state,  empty_init, "Castle Toy", "Name That Tune (Castle Toy)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***

CONS( 1980, invspace, 0,        0, invspace, invspace, invspace_state, empty_init, "Epoch", "Invader From Space", MACHINE_SUPPORTS_SAVE )
CONS( 1980, efball,   0,        0, efball,   efball,   efball_state,   empty_init, "Epoch", "Electronic Football (Epoch)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, galaxy2,  0,        0, galaxy2,  galaxy2,  galaxy2_state,  empty_init, "Epoch", "Galaxy II (VFD Rev. D)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, galaxy2b, galaxy2,  0, galaxy2b, galaxy2,  galaxy2_state,  empty_init, "Epoch", "Galaxy II (VFD Rev. B)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, astrocmd, 0,        0, astrocmd, astrocmd, astrocmd_state, empty_init, "Epoch", "Astro Command", MACHINE_SUPPORTS_SAVE )
CONS( 1982, edracula, 0,        0, edracula, edracula, edracula_state, empty_init, "Epoch", "Dracula (Epoch)", MACHINE_SUPPORTS_SAVE )

CONS( 1979, mcompgin, 0,        0, mcompgin, mcompgin, mcompgin_state, empty_init, "Mattel Electronics", "Computer Gin", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1979, mvbfree,  0,        0, mvbfree,  mvbfree,  mvbfree_state,  empty_init, "Mego", "Mini-Vid: Break Free", MACHINE_SUPPORTS_SAVE )

CONS( 1980, grobot9,  0,        0, grobot9,  grobot9,  grobot9_state,  empty_init, "Takatoku Toys", "Game Robot 9", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // some of the minigames: ***

CONS( 1980, tccombat, 0,        0, tccombat, tccombat, tccombat_state, empty_init, "Tomy", "Cosmic Combat", MACHINE_SUPPORTS_SAVE )
CONS( 1980, tmtennis, 0,        0, tmtennis, tmtennis, tmtennis_state, empty_init, "Tomy", "Tennis (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, tmpacman, 0,        0, tmpacman, tmpacman, tmpacman_state, empty_init, "Tomy", "Pac Man (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, tmscramb, 0,        0, tmscramb, tmscramb, tmscramb_state, empty_init, "Tomy", "Scramble (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, tcaveman, 0,        0, tcaveman, tcaveman, tcaveman_state, empty_init, "Tomy", "Caveman (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1984, alnchase, 0,        0, alnchase, alnchase, alnchase_state, empty_init, "Tomy", "Alien Chase", MACHINE_SUPPORTS_SAVE )

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, book, playing cards, pen & paper, etc.
