// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton
/***************************************************************************

Hitachi HMCS40 MCU tabletops/handhelds or other simple devices,
most of them are VFD electronic games/toys.

known chips:

  serial  device   etc.
----------------------------------------------------------------
 @A07     HD38750  1979, Bambino Knock-Em Out Boxing (ET-06B)
 @A08     HD38750  1979, Bambino Dribble Away Basketball (ET-05)
 @A45     HD38750  1981, VTech Invaders
 *A56     HD38750  1981, Actronics(Hanzawa) Twinvader (small brown version)
 *A58     HD38750  1981, Actronics(Hanzawa) Challenge Racer/Ludotronic(Hanzawa) Grand Prix Turbo
 *A62     HD38750  1982, Actronics(Hanzawa) Pack'n Maze
 *A67     HD38750  1982, Romtec Pucki & Monsters (ET-803)

 @A04     HD38800  1980, Gakken Heiankyo Alien
  A16     HD38800  1981, Entex Select-A-Game cartridge: Basketball 3 -> sag.cpp
 *A20     HD38800  1981, Entex Super Space Invader 2
 @A25     HD38800  1981, Coleco Alien Attack
 @A27     HD38800  1981, Bandai Packri Monster
 @A31     HD38800  1981, Entex Select-A-Game cartridge: Space Invader 2 -> sag.cpp - also used in 2nd version of Super Space Invader 2!
  A36     HD38800  1981, Entex Select-A-Game cartridge: Pac-Man 2       -> "
  A37     HD38800  1981, Entex Select-A-Game cartridge: Baseball 4      -> "
  A38     HD38800  1981, Entex Select-A-Game cartridge: Pinball         -> "
 *A41     HD38800  1982, Gakken Puck Monster
 *A42     HD38800  1981, Akai GX-77
 *A51     HD38800  1981, Actronics(Hanzawa) Twinvader (larger white version)
 @A70     HD38800  1982, Coleco Galaxian
 @A73     HD38800  1982, Mattel Star Hawk (PT-317B)
 @A77     HD38800  1982, Bandai Frisky Tom (PT-327A)
 @A88     HD38800  1982, Tomy Tron (THN-02)
 @B01     HD38800  1982, Gakken Crazy Kong
 @B19     HD38800  1982, Bandai Zaxxon
 @B23     HD38800  1982, Tomy Kingman (THF-01II)
 *B24     HD38800  1982, Actronics(Hanzawa) Wanted G-Man
 *B29     HD38800  1984, Tomy Portable 6000 Bombman
 *B31     HD38800  1983, Romtec Frog Prince (ET-806)
 *B35     HD38800  1983, Bandai Gundam vs Gelgoog Zaku
 *B42     HD38800  1983, Bandai Kiteyo Parman
 @B43     HD38800  1983, Bandai Dokodemo Dorayaki Doraemon (PT-412)
 *B48     HD38800  1983, Bandai Go Go Dynaman
 @B52     HD38800  1983, Bandai Ultraman Monster Battle (PT-424)

 @A09     HD38820  1980, Mattel World Championship Baseball
 @A13     HD38820  1981, Entex Galaxian 2
 @A23     HD38820  1981, Entex Pac Man 2
 @A28     HD38820  1981, Coleco Pac-Man (ver 1)
 @A29     HD38820  1981, Coleco Pac-Man (ver 2)
 @A32     HD38820  1982, Gakken Super Cobra
 *A38     HD38820  1982, Entex Crazy Climber
 @A42     HD38820  1982, Entex Stargate
 @A43     HD38820  1982, Entex Turtles
 @A45     HD38820  1982, Coleco Donkey Kong
 @A49     HD38820  1983, Bandai Zackman
 @A61     HD38820  1983, Coleco Ms. Pac-Man
 *A62     HD38820  1983, Coleco Zaxxon
 @A63     HD38820  1983, Bandai Pengo
 @A65     HD38820  1983, Bandai Burger Time (PT-389)
 @A69     HD38820  1983, Gakken Dig Dug
 @A70     HD38820  1983, Parker Brothers Q*Bert
 *A75     HD38820  1983, Bandai Toukon Juohmaru
 @A85     HD38820  1984, Bandai Machine Man (PT-438)
 @A88     HD38820  1984, Bandai Pair Match (PT-460) (1/2)
 @A89     HD38820  1984, Bandai Pair Match (PT-460) (2/2)

  A34     HD44801  1981, SciSys Mini Chess -> saitek_minichess.cpp
  A50     HD44801  1981, CXG Sensor Computachess -> cxg_scptchess.cpp
  A75     HD44801  1982, Alpha 8201 protection MCU -> machine/alpha8201.*
 *A85     HD44801  1982, SciSys Travel Sensor / Travel Mate / Chesspartner 5000/6000
 *A92     HD44801  1982, SciSys Play Bridge Computer
  B35     HD44801  1983, Alpha 8302 protection MCU (see 8201)
  B42     HD44801  1983, Alpha 8303 protection MCU (see 8201)
 *B43     HD44801  1983, Alpha 8304 protection MCU (see 8201)
  C57     HD44801  1985, Alpha 8505 protection MCU (see 8201)
  C89     HD44801  1985, CXG Portachess (1985 version) -> cxg_scptchess.cpp

 *A86     HD44820  1983, Chess King Pocket Micro
 *B63     HD44820  1985, CXG Pocket Chess (12 buttons)

 *A14     HD44840  1982, CXG Computachess II / Advanced Portachess

 *B55     HD44860  1987, Saitek Pro Bridge 100

 *A04     HD44868  1984, SciSys Rapier
 *A07     HD44868  1984, Chess King Pocket Micro Deluxe
 *A12     HD44868  1985, SciSys MK 10 / Pocket Chess
 *A14     HD44868  1985, SciSys Kasparov Plus

  (* means undumped unless noted, @ denotes it's in this driver)

============================================================================

ROM source notes when dumped from another publisher, but confident it's the same:
- gckong: CGL Super Kong
- ghalien: CGL Earth Invaders
- kingman: Tandy Kingman
- zackman: Tandy Zackman

TODO:
- cgalaxn discrete sound (alien attacking sound effect)
- gckong glitchy jump on 1st level (rarely happens), caused by MCU stack overflow.
  It can be tested by jumping up repeatedly at the start position under the ladder,
  if the glitch happens there, you can jump onto the 2nd floor.
- epacman2 booting the game in demo mode, pacman should take the shortest route to
  the upper-left power pill: mcu cycle/interrupt timing related
- kevtris's HMCS40 ROM dumps are incomplete, missing MCU factory test code from
  the 2nd half of the ROM, none of the games access it though and it's impossible
  to execute unless the chip is in testmode.
- Though very uncommon when compared to games with LED/lamp display, some
  games may manipulate VFD plate brightness by strobing it longer/shorter,
  eg. cgalaxn when a ship explodes.
- bzaxxon 3D effect is difficult to simulate
- improve/redo SVGs of: bzaxxon, bbtime
- get rid of hardcoded color overlay from SVGs, use MAME internal artwork

***************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "cpu/cop400/cop400.h"
#include "video/pwm.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

// internal artwork (complete)
#include "pairmtch.lh"

// internal artwork (bezel overlay)
#include "bambball.lh"
#include "gckong.lh"
#include "mwcbaseb.lh"
#include "msthawk.lh"
#include "packmon.lh"

//#include "hh_hmcs40_test.lh" // common test-layout - no svg artwork(yet), use external artwork


class hh_hmcs40_state : public driver_device
{
public:
	hh_hmcs40_state(const machine_config &mconfig, device_type type, const char *tag) :
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
	required_device<hmcs40_cpu_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<7> m_inputs; // max 7

	// misc common
	u8 m_r[8] = { };                // MCU R ports write data (optional)
	u16 m_d = 0;                    // MCU D port write data (optional)
	u8 m_int[2] = { };              // MCU INT0/1 pins state
	u16 m_inp_mux = 0;              // multiplexed inputs mask

	u32 m_grid = 0;                 // VFD current row data
	u64 m_plate = 0;                // VFD current column data

	u16 read_inputs(int columns);
	void refresh_interrupts(void);
	void set_interrupt(int line, int state);
};


// machine start/reset

void hh_hmcs40_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_r));
	save_item(NAME(m_int));
	save_item(NAME(m_d));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_hmcs40_state::machine_reset()
{
	refresh_interrupts();
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u16 hh_hmcs40_state::read_inputs(int columns)
{
	u16 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}


// interrupt handling

void hh_hmcs40_state::refresh_interrupts()
{
	for (int i = 0; i < 2; i++)
		m_maincpu->set_input_line(i, m_int[i] ? ASSERT_LINE : CLEAR_LINE);
}

void hh_hmcs40_state::set_interrupt(int line, int state)
{
	line = line ? 1 : 0;
	state = state ? 1 : 0;

	if (state != m_int[line])
	{
		if (machine().phase() >= machine_phase::RESET)
			m_maincpu->set_input_line(line, state ? ASSERT_LINE : CLEAR_LINE);
		m_int[line] = state;
	}
}

INPUT_CHANGED_MEMBER(hh_hmcs40_state::single_interrupt_line)
{
	set_interrupt((int)param, newval);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Bambino Dribble Away Basketball (manufactured in Japan)
  * PCB label Emix Corp. ET-05
  * Hitachi HD38750A08 MCU
  * cyan VFD display Emix-106, with bezel overlay

***************************************************************************/

class bambball_state : public hh_hmcs40_state
{
public:
	bambball_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bambball(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u8 input_r();
};

// handlers

void bambball_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R3x(,D0-D3): vfd plate
	int shift = (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 plate = bitswap<16>(m_plate,13,8,4,12,9,10,14,1,7,0,15,11,6,3,5,2);
	m_display->matrix(m_grid, plate);
}

void bambball_state::grid_w(u16 data)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D7-D10: input mux
	m_inp_mux = data >> 7 & 0xf;

	// D7-D15: vfd grid
	m_grid = data >> 7 & 0x1ff;

	// D0-D3: more plates (update display there)
	plate_w(3 + 1, data & 0xf);
}

u8 bambball_state::input_r()
{
	// R0x: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( bambball )
	PORT_START("IN.0") // D7 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Dribble Low")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Dribble Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Dribble High")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Shoot")

	PORT_START("IN.1") // D8 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // D9 port R0x
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Display")

	PORT_START("IN.3") // D10 port R0x
	PORT_CONFNAME( 0x07, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void bambball_state::bambball(machine_config &config)
{
	// basic machine hardware
	HD38750(config, m_maincpu, 400000); // approximation
	m_maincpu->read_r<0>().set(FUNC(bambball_state::input_r));
	m_maincpu->write_r<1>().set(FUNC(bambball_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bambball_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bambball_state::plate_w));
	m_maincpu->write_d().set(FUNC(bambball_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 478);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 16);
	config.set_default_layout(layout_bambball);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bambball )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a08", 0x0000, 0x0800, CRC(907fef18) SHA1(73fe7ca7c6332268a3a9abc5ac88ada2991012fb) )
	ROM_CONTINUE(           0x0f00, 0x0080 )

	ROM_REGION( 281988, "screen", 0)
	ROM_LOAD( "bambball.svg", 0, 281988, CRC(63019194) SHA1(cbfb5b051d8f57f6b4d698796030850b3631ed56) )
ROM_END





/***************************************************************************

  Bambino Knock-Em Out Boxing
  * PCB label Emix Corp. ET-06B
  * Hitachi HD38750A07 MCU
  * cyan VFD display Emix-103, with blue or green color overlay

***************************************************************************/

class bmboxing_state : public hh_hmcs40_state
{
public:
	bmboxing_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bmboxing(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u8 input_r();
};

// handlers

void bmboxing_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,0,1,2,3,4,5,6,7,8);
	u32 plate = bitswap<16>(m_plate,15,14,13,12,1,2,0,3,11,4,10,7,5,6,9,8);
	m_display->matrix(grid, plate);
}

void bmboxing_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R3x: vfd plate
	int shift = (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bmboxing_state::grid_w(u16 data)
{
	// D13: speaker out
	m_speaker->level_w(data >> 13 & 1);

	// D9-D12: input mux
	m_inp_mux = data >> 9 & 0xf;

	// D4-D12: vfd grid
	m_grid = data >> 4 & 0x1ff;
	update_display();
}

u8 bmboxing_state::input_r()
{
	// R0x: multiplexed inputs
	return read_inputs(4);
}

// config

/* physical button layout and labels are like this:

    * left = P2 side *                                       * right = P1 side *

    [ BACK ]  [ HIGH ]        (players sw)                   [ HIGH ]  [ BACK ]
                              1<--->2         [START/
    [NORMAL]  [MEDIUM]                         RESET]        [MEDIUM]  [NORMAL]
                              1<---OFF--->2
    [ DUCK ]  [ LOW  ]        (skill lvl sw)                 [ LOW  ]  [ DUCK ]
*/

static INPUT_PORTS_START( bmboxing )
	PORT_START("IN.0") // D9 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P1 Punch High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 Punch Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("P1 Punch Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // D10 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P1 Position Normal")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 Position Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P1 Position Ducking")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // D11 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("P2 Punch High")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 Punch Medium")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("P2 Punch Low")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // D12 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 Position Normal")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("P2 Position Back")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("P2 Position Ducking")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // port D
	PORT_CONFNAME( 0x0001, 0x0000, DEF_STR( Players ) )
	PORT_CONFSETTING(      0x0000, "1" )
	PORT_CONFSETTING(      0x0001, "2" )
	PORT_CONFNAME( 0x0002, 0x0000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x0000, "1" )
	PORT_CONFSETTING(      0x0002, "2" )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void bmboxing_state::bmboxing(machine_config &config)
{
	// basic machine hardware
	HD38750(config, m_maincpu, 400000); // approximation
	m_maincpu->read_r<0>().set(FUNC(bmboxing_state::input_r));
	m_maincpu->write_r<1>().set(FUNC(bmboxing_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bmboxing_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bmboxing_state::plate_w));
	m_maincpu->write_d().set(FUNC(bmboxing_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.4");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 529);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 12);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bmboxing )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a07", 0x0000, 0x0800, CRC(7f33e259) SHA1(c5fcdd6bf060c96666354f09f0570c754f6ed4e0) )
	ROM_CONTINUE(           0x0f00, 0x0080 )

	ROM_REGION( 257144, "screen", 0)
	ROM_LOAD( "bmboxing.svg", 0, 257144, CRC(dab81477) SHA1(28b0c844a311e2023ffa71d754e799059b7d050f) )
ROM_END





/***************************************************************************

  Bandai Frisky Tom (manufactured in Japan)
  * PCB label Kaken Corp., PT-327A
  * Hitachi HD38800A77 MCU
  * cyan/red/green VFD display Futaba DM-43ZK 2E

***************************************************************************/

class bfriskyt_state : public hh_hmcs40_state
{
public:
	bfriskyt_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bfriskyt(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int1(); }

private:
	void update_int1();
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void bfriskyt_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,8,0,1,2,3,4,5,6,7);
	u32 plate = bitswap<24>(m_plate,23,22,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21);
	m_display->matrix(grid, plate);
}

void bfriskyt_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bfriskyt_state::grid_w(u16 data)
{
	// D6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// D11-D15: input mux
	u8 inp_mux = data >> 11 & 0x1f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D8-D15: vfd grid
	m_grid = data >> 8 & 0xff;

	// D0-D5: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x3f0000);
	update_display();
}

void bfriskyt_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(5));
}

// config

static INPUT_PORTS_START( bfriskyt )
	PORT_START("IN.0") // D11 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, 0)

	PORT_START("IN.1") // D12 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, 0)

	PORT_START("IN.2") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, 0)

	PORT_START("IN.3") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, 0)

	PORT_START("IN.4") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bfriskyt_state, input_changed, 0)

	PORT_START("IN.5") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)
INPUT_PORTS_END

void bfriskyt_state::bfriskyt(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(bfriskyt_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(bfriskyt_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bfriskyt_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bfriskyt_state::plate_w));
	m_maincpu->write_d().set(FUNC(bfriskyt_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 675);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 22);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bfriskyt )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a77", 0x0000, 0x1000, CRC(a2445c4f) SHA1(0aaccfec90b66d27dae194d4462d88e654c41578) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 413577, "screen", 0)
	ROM_LOAD( "bfriskyt.svg", 0, 413577, CRC(17090264) SHA1(4512a8a91a459f2ddc258641c6d38c2f48f4160f) )
ROM_END





/***************************************************************************

  Bandai Packri Monster (manufactured in Japan)
  * PCB label DM-21ZA2
  * Hitachi HD38800A27 MCU
  * cyan/red/green VFD display Futaba DM-21ZK 2B, with bezel overlay

  known releases:
  - Japan: FL Packri Monster, published by Bandai
  - USA(World?): Packri Monster, published by Bandai
  - USA/Canada: Hungry Monster, published by Tandy
  - other: Gobble Man/Ogre Monster, published by Tandy

***************************************************************************/

class packmon_state : public hh_hmcs40_state
{
public:
	packmon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void packmon(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u16 input_r();
};

// handlers

void packmon_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D0-D3): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,0,1,2,3,4,5,6,19,18,17,16,15,14,13,12,11,10,9,8,7);
	m_display->matrix(grid, plate);
}

void packmon_state::grid_w(u16 data)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D11-D15: input mux
	m_inp_mux = data >> 11 & 0x1f;

	// D6-D15: vfd grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D3: plate 9-12 (update display there)
	plate_w(4, data & 0xf);
}

u16 packmon_state::input_r()
{
	// D5: multiplexed inputs
	return read_inputs(5) & 0x20;
}

// config

static INPUT_PORTS_START( packmon )
	PORT_START("IN.0") // D11 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START )

	PORT_START("IN.1") // D12 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D13 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.3") // D14 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.4") // D15 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
INPUT_PORTS_END

void packmon_state::packmon(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(packmon_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(packmon_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(packmon_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(packmon_state::plate_w));
	m_maincpu->write_d().set(FUNC(packmon_state::grid_w));
	m_maincpu->read_d().set(FUNC(packmon_state::input_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 680);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 20);
	config.set_default_layout(layout_packmon);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( packmon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a27", 0x0000, 0x1000, CRC(86e09e84) SHA1(ac7d3c43667d5720ca513f8ff51d146d9f2af124) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 224386, "screen", 0)
	ROM_LOAD( "packmon.svg", 0, 224386, CRC(b2ee5b6b) SHA1(e53b4d5a4118cc5fbec4656580c2aab76af8f8d7) )
ROM_END





/***************************************************************************

  Bandai Zaxxon (manufactured in Japan, licensed from Sega)
  * PCB label FL Zaxxon
  * Hitachi HD38800B19 MCU
  * cyan/red/blue VFD display NEC FIP11BM24T no. 4-8, half of it reflected
    with a one-way mirror to give the illusion of a 3D display

***************************************************************************/

class bzaxxon_state : public hh_hmcs40_state
{
public:
	bzaxxon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bzaxxon(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int1(); }

private:
	void update_int1();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void bzaxxon_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D0-D2): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,6,7,8,9,10,5,4,3,2,1,0);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,5,7,0,1,2,3,4,6,19,16,17,18,15,14,13,12,10,8,9,11) | 0x800;
	m_display->matrix(grid, plate);
}

void bzaxxon_state::grid_w(u16 data)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D7-D10: input mux
	u8 inp_mux = data >> 7 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D5-D15: vfd grid
	m_grid = data >> 5 & 0x7ff;

	// D0-D2: plate 7-9 (update display there)
	plate_w(4, data & 7);
}

void bzaxxon_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(4));
}

// config

static INPUT_PORTS_START( bzaxxon )
	PORT_START("IN.0") // D7 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, 0)

	PORT_START("IN.1") // D8 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, 0)

	PORT_START("IN.2") // D9 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, 0)

	PORT_START("IN.3") // D10 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bzaxxon_state, input_changed, 0)

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.5") // port D
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0xfff7, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void bzaxxon_state::bzaxxon(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 450000); // approximation
	m_maincpu->write_r<0>().set(FUNC(bzaxxon_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(bzaxxon_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bzaxxon_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bzaxxon_state::plate_w));
	m_maincpu->write_d().set(FUNC(bzaxxon_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.5");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(613, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(11, 20);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bzaxxon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b19", 0x0000, 0x1000, CRC(4fecb80d) SHA1(7adf079480ffd3825ad5ae1eaa4d892eecbcc42d) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 521080, "screen", 0)
	ROM_LOAD( "bzaxxon.svg", 0, 521080, BAD_DUMP CRC(f4fbb2de) SHA1(83db400e67d91ae4bfee3e8568ae9df94ebede19) )
ROM_END





/***************************************************************************

  Bandai Zackman "The Pit, FL Exploration of Space" (manufactured in Japan)
  * Hitachi QFP HD38820A49 MCU
  * cyan/red/yellow VFD display Futaba DM-53Z 3E, with color overlay

  known releases:
  - World: Zackman, published by Bandai
  - USA: Zackman, published by Tandy

***************************************************************************/

class zackman_state : public hh_hmcs40_state
{
public:
	zackman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void zackman(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int0(); }

private:
	void update_int0();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void zackman_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R6x(,D0,D1): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,6,7);
	u32 plate = bitswap<32>(m_plate,31,30,27,0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,29,28,23,22,21,20,19,18,17,16,15,14,13,12);
	m_display->matrix(grid, plate);
}

void zackman_state::grid_w(u16 data)
{
	// D2: speaker out
	m_speaker->level_w(data >> 2 & 1);

	// D11-D14: input mux
	u8 inp_mux = data >> 11 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D8-D15: vfd grid
	m_grid = data >> 8 & 0xff;

	// D0,D1: plate 12,13 (update display there)
	plate_w(7, data & 3);
}

void zackman_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(4));
}

// config

static INPUT_PORTS_START( zackman )
	PORT_START("IN.0") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, 0)

	PORT_START("IN.1") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, 0)

	PORT_START("IN.2") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, 0)

	PORT_START("IN.3") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, zackman_state, input_changed, 0)

	PORT_START("IN.4") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1)
INPUT_PORTS_END

void zackman_state::zackman(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(zackman_state::plate_w));
	m_maincpu->write_d().set(FUNC(zackman_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(487, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 29);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( zackman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a49", 0x0000, 0x1000, CRC(b97f5ef6) SHA1(7fe20e8107361caf9ea657e504be1f8b10b8b03f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 910689, "screen", 0)
	ROM_LOAD( "zackman.svg", 0, 910689, CRC(5f322820) SHA1(4210aff160e5de9a409aba8b915aaebff2a92647) )
ROM_END





/***************************************************************************

  Bandai Pengo (manufactured in Japan, licensed from Sega)
  * PCB label FL Pengo(in katakana)
  * Hitachi QFP HD38820A63 MCU
  * cyan/red/blue VFD display Futaba DM-68ZK 3D DM-63

***************************************************************************/

class bpengo_state : public hh_hmcs40_state
{
public:
	bpengo_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bpengo(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int0(); }

private:
	void update_int0();
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void bpengo_state::update_display()
{
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,6,7);
	u32 plate = bitswap<32>(m_plate,31,30,29,28,23,22,21,16,17,18,19,20,27,26,25,24,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
	m_display->matrix(grid, plate);
}

void bpengo_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R6x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bpengo_state::grid_w(u16 data)
{
	// D10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// D12-D15: input mux
	u8 inp_mux = data >> 12 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D0-D7: vfd grid
	m_grid = data & 0xff;
	update_display();
}

void bpengo_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(4));
}

// config

static INPUT_PORTS_START( bpengo )
	PORT_START("IN.0") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, 0)

	PORT_START("IN.1") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, 0)

	PORT_START("IN.2") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, 0)

	PORT_START("IN.3") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bpengo_state, input_changed, 0)

	PORT_START("IN.4") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1)

	PORT_START("IN.5") // port D
	PORT_CONFNAME( 0x0800, 0x0000, "Factory Test" )
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0800, DEF_STR( On ) )
	PORT_BIT( 0xf7ff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void bpengo_state::bpengo(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(bpengo_state::plate_w));
	m_maincpu->write_d().set(FUNC(bpengo_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.5");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 759);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 25);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bpengo )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a63", 0x0000, 0x1000, CRC(ebd6bc64) SHA1(0a322c47b9553a2739a85908ce64b9650cf93d49) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 565069, "screen", 0)
	ROM_LOAD( "bpengo.svg", 0, 565069, CRC(fb25ffeb) SHA1(fb4db5120f1e35e39c7fb2da4af3aa63417efaf1) )
ROM_END





/***************************************************************************

  Bandai Burger Time (manufactured in Japan, licensed from Data East)
  * PCB label Kaken Corp. PT-389 Burger Time
  * Hitachi QFP HD38820A65 MCU
  * cyan/red/green VFD display NEC FIP6AM25T no. 21-21

***************************************************************************/

class bbtime_state : public hh_hmcs40_state
{
public:
	bbtime_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bbtime(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int0(); }

private:
	void update_int0();
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void bbtime_state::update_display()
{
	u8 grid = bitswap<8>(m_grid,7,6,0,1,2,3,4,5);
	u32 plate = bitswap<32>(m_plate,31,30,29,28,25,24,26,27,22,23,15,14,12,11,10,8,7,6,4,1,5,9,13,3,2,16,17,18,19,20,0,21) | 0x1;
	m_display->matrix(grid, plate);
}

void bbtime_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R6x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void bbtime_state::grid_w(u16 data)
{
	// D3: speaker out
	m_speaker->level_w(data >> 3 & 1);

	// D10-D14: input mux
	u8 inp_mux = data >> 10 & 0x1f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D4-D9: vfd grid
	m_grid = data >> 4 & 0x3f;
	update_display();
}

void bbtime_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(5));
}

// config

static INPUT_PORTS_START( bbtime )
	PORT_START("IN.0") // D10 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, 0)

	PORT_START("IN.1") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, 0)

	PORT_START("IN.2") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, 0)

	PORT_START("IN.3") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, 0)

	PORT_START("IN.4") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, bbtime_state, input_changed, 0)

	PORT_START("IN.5") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1)
INPUT_PORTS_END

void bbtime_state::bbtime(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(bbtime_state::plate_w));
	m_maincpu->write_d().set(FUNC(bbtime_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(379, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(6, 28);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bbtime )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a65", 0x0000, 0x1000, CRC(33611faf) SHA1(29b6a30ed543688d31ec2aa18f7938fa4eef30b0) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 461605, "screen", 0)
	ROM_LOAD( "bbtime.svg", 0, 461605, BAD_DUMP CRC(5b335271) SHA1(46c45b711358e8397ae707668aecead9e341ab8a) )
ROM_END





/***************************************************************************

  Bandai Dokodemo Dorayaki Doraemon (FL LSI Game Push Up) (manufactured in Japan)
  * PCB label Kaken Corp PT-412 FL-Doreamon(in katakana)
  * Hitachi HD38800B43 MCU
  * cyan/red/blue VFD display Futaba DM-71

***************************************************************************/

class bdoramon_state : public hh_hmcs40_state
{
public:
	bdoramon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bdoramon(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void bdoramon_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D0-D3): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,7,6);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,11,19,18,17,16,15,14,13,12,10,9,8,7,6,5,4,3,2,1,0);
	m_display->matrix(grid, plate);
}

void bdoramon_state::grid_w(u16 data)
{
	// D7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// D8-D15: vfd grid
	m_grid = data >> 8 & 0xff;

	// D0-D3: plate 15-18 (update display there)
	plate_w(4, data & 0xf);
}

// config

static INPUT_PORTS_START( bdoramon )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.1") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1)

	PORT_START("IN.2") // port D
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xff8f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // port R2x
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

void bdoramon_state::bdoramon(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(bdoramon_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(bdoramon_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bdoramon_state::plate_w));
	m_maincpu->read_r<2>().set_ioport("IN.3");
	m_maincpu->write_r<3>().set(FUNC(bdoramon_state::plate_w));
	m_maincpu->write_d().set(FUNC(bdoramon_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 668);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 19);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bdoramon )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b43", 0x0000, 0x1000, CRC(9387ca42) SHA1(8937e208934b34bd9f49700aa50287dfc8bda76c) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 624751, "screen", 0)
	ROM_LOAD( "bdoramon.svg", 0, 624751, CRC(5dc4017c) SHA1(2091765de401969651b8eb22067572be72d12398) )
ROM_END





/***************************************************************************

  Bandai Ultraman Monster Battle (FL LSI Game Push Up) (manufactured in Japan)
  * PCB label Kaken Corp. PT-424 FL Ultra Man
  * Hitachi HD38800B52 MCU
  * cyan/red/blue VFD display NEC FIP8BM25T no. 21-8 2

***************************************************************************/

class bultrman_state : public hh_hmcs40_state
{
public:
	bultrman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void bultrman(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void bultrman_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D0-D2): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,6,7);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,0,18,17,16,15,14,13,12,3,11,10,9,8,7,6,5,4,1,2);
	m_display->matrix(grid, plate);
}

void bultrman_state::grid_w(u16 data)
{
	// D7: speaker out
	m_speaker->level_w(data >> 7 & 1);

	// D8-D15: vfd grid
	m_grid = data >> 8 & 0xff;

	// D0-D2: plate 15-17 (update display there)
	plate_w(4, data & 7);
}

// config

static INPUT_PORTS_START( bultrman )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.1") // port D
	PORT_CONFNAME( 0x0010, 0x0000, "Factory Test" )
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0010, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xff8f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void bultrman_state::bultrman(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 350000); // approximation
	m_maincpu->write_r<0>().set(FUNC(bultrman_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(bultrman_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(bultrman_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(bultrman_state::plate_w));
	m_maincpu->write_d().set(FUNC(bultrman_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 673);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 18);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( bultrman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b52", 0x0000, 0x1000, CRC(88d372dc) SHA1(f2ac3b89be8afe6fb65914ccebe1a56316b9472a) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 405717, "screen", 0)
	ROM_LOAD( "bultrman.svg", 0, 405717, CRC(13367971) SHA1(f294898712d1e146ff267bb1e3cfd059f972b248) )
ROM_END





/***************************************************************************

  Bandai Machine Man (FL Flat Type) (manufactured in Japan)
  * PCB label Kaken PT-438
  * Hitachi QFP HD38820A85 MCU
  * cyan/red/green VFD display NEC FIP5CM33T no. 4 21

***************************************************************************/

class machiman_state : public hh_hmcs40_state
{
public:
	machiman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void machiman(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void machiman_state::update_display()
{
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18);
	m_display->matrix(m_grid, plate);
}

void machiman_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x,R6012: vfd plate
	int shift = (offset == 6) ? 16 : offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void machiman_state::grid_w(u16 data)
{
	// D13: speaker out
	m_speaker->level_w(data >> 13 & 1);

	// D0-D4: vfd grid
	m_grid = data & 0x1f;
	update_display();
}

// config

static INPUT_PORTS_START( machiman )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.1") // port D
	PORT_BIT( 0x3fff, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
INPUT_PORTS_END

void machiman_state::machiman(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(machiman_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(machiman_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(machiman_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(machiman_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(machiman_state::plate_w));
	m_maincpu->write_d().set(FUNC(machiman_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1534, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(5, 19);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( machiman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a85", 0x0000, 0x1000, CRC(894b4954) SHA1(cab49638a326b031aa548301beb16f818759ef62) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 374097, "screen", 0)
	ROM_LOAD( "machiman.svg", 0, 374097, CRC(78af02ac) SHA1(1b4bbea3e46e1bf33149727d9725bc9b18652b9c) )
ROM_END





/***************************************************************************

  Bandai Pair Match (manufactured in Japan)
  * PCB label Kaken Corp. PT-460
  * Hitachi QFP HD38820A88 MCU(main), HD38820A89(audio)
  * cyan/red VFD display

  This is a memory game, the difference is instead of pictures, the player
  needs to match sound effects. It has an extra MCU for sound. The case is
  shaped like a glossy black pyramid. Star Trek fans will recognize it as
  a prop used in TNG Ten Forward.

***************************************************************************/

class pairmtch_state : public hh_hmcs40_state
{
public:
	pairmtch_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch%u", 0)
	{ }

	void pairmtch(machine_config &config);

private:
	required_device<hmcs40_cpu_device> m_audiocpu;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u8 input_r();

	void sound_w(u8 data);
	void sound2_w(u8 data);
	void speaker_w(u16 data);
};

// handlers: maincpu side

void pairmtch_state::update_display()
{
	m_display->matrix(m_grid, m_plate);
}

void pairmtch_state::plate_w(offs_t offset, u8 data)
{
	// R2x,R3x,R6x: vfd plate
	int shift = (offset == 6) ? 8 : (offset-2) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void pairmtch_state::grid_w(u16 data)
{
	// D7: sound reset (to audiocpu reset line)
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	// D9: sound start (to audiocpu INT0)
	m_audiocpu->set_input_line(0, (data & 0x200) ? ASSERT_LINE : CLEAR_LINE);

	// D10,D15: input mux
	m_inp_mux = (data >> 10 & 1) | (data >> 14 & 2);

	// D0-D5: vfd grid
	m_grid = data & 0x3f;
	update_display();
}

u8 pairmtch_state::input_r()
{
	// R4x: multiplexed inputs
	return read_inputs(2);
}

void pairmtch_state::sound_w(u8 data)
{
	// R5x: soundlatch (to audiocpu R2x)
	m_soundlatch[0]->write(bitswap<8>(data,7,6,5,4,0,1,2,3));
}

// handlers: audiocpu side

void pairmtch_state::sound2_w(u8 data)
{
	// R2x: soundlatch (to maincpu R5x)
	m_soundlatch[1]->write(bitswap<8>(data,7,6,5,4,0,1,2,3));
}

void pairmtch_state::speaker_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1: sound ack (to maincpu INT0)
	m_maincpu->set_input_line(0, (data & 2) ? ASSERT_LINE : CLEAR_LINE);
}

// config

static INPUT_PORTS_START( pairmtch )
	PORT_START("IN.0") // D10 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.1") // D15 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.2") // port D
	PORT_CONFNAME( 0x0040, 0x0000, "Factory Test" )
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0040, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_CONFNAME( 0x0800, 0x0800, DEF_STR( Players ) )
	PORT_CONFSETTING(      0x0800, "1" )
	PORT_CONFSETTING(      0x0000, "2" )
	PORT_CONFNAME( 0x3000, 0x2000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x2000, "1" )
	PORT_CONFSETTING(      0x1000, "2" )
	PORT_CONFSETTING(      0x0000, "3" )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x86bf, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pairmtch_state::pairmtch(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<2>().set(FUNC(pairmtch_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(pairmtch_state::plate_w));
	m_maincpu->read_r<4>().set(FUNC(pairmtch_state::input_r));
	m_maincpu->write_r<5>().set(FUNC(pairmtch_state::sound_w));
	m_maincpu->read_r<5>().set(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	m_maincpu->write_r<6>().set(FUNC(pairmtch_state::plate_w));
	m_maincpu->write_d().set(FUNC(pairmtch_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.2");

	HD38820(config, m_audiocpu, 400000); // approximation
	m_audiocpu->write_r<2>().set(FUNC(pairmtch_state::sound2_w));
	m_audiocpu->read_r<2>().set(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	m_audiocpu->write_d().set(FUNC(pairmtch_state::speaker_w));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 12);
	config.set_default_layout(layout_pairmtch);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);
}

// roms

ROM_START( pairmtch )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a88", 0x0000, 0x1000, CRC(ffa35730) SHA1(5a80b9025aaad2ac0ab0b1436a1355ae8cd3f868) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 0x2000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a89", 0x0000, 0x1000, CRC(3533ec56) SHA1(556d69e78a0ee1bf766fce16ed58992d7272d57f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )
ROM_END





/***************************************************************************

  Coleco Alien Attack (manufactured in Taiwan)
  * Hitachi HD38800A25 MCU
  * cyan/red VFD display Futaba DM-19Z 1J

  It looks like Coleco took Gakken's Heiankyo Alien and turned it into a more
  action-oriented game.

***************************************************************************/

class alnattck_state : public hh_hmcs40_state
{
public:
	alnattck_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void alnattck(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u16 input_r();
};

// handlers

void alnattck_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D0-D3): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,18,17,16,11,9,8,10,7,2,0,1,3,4,5,6,12,13,14,15);
	m_display->matrix(m_grid, plate);
}

void alnattck_state::grid_w(u16 data)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D7-D13: input mux
	m_inp_mux = data >> 7 & 0x7f;

	// D6-D15: vfd grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D3: plate 16-19 (update display there)
	plate_w(4, data & 0xf);
}

u16 alnattck_state::input_r()
{
	// D5: multiplexed inputs
	return read_inputs(7) & 0x20;
}

// config

static INPUT_PORTS_START( alnattck )
	PORT_START("IN.0") // D7 line D5
	PORT_CONFNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x20, "2" )

	PORT_START("IN.1") // D8 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D9 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.3") // D10 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.4") // D11 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN.5") // D12 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Move")

	PORT_START("IN.6") // D13 line D5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Fire")
INPUT_PORTS_END

void alnattck_state::alnattck(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(alnattck_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(alnattck_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(alnattck_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(alnattck_state::plate_w));
	m_maincpu->write_d().set(FUNC(alnattck_state::grid_w));
	m_maincpu->read_d().set(FUNC(alnattck_state::input_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 700);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 20);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( alnattck )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a25", 0x0000, 0x1000, CRC(18b50869) SHA1(11e9d5f7b4ae818b077b0ee14a3b43190e20bff3) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 564271, "screen", 0)
	ROM_LOAD( "alnattck.svg", 0, 564271, CRC(5466d1d4) SHA1(3295272015969e58fddc53272769e1fc1bd4b355) )
ROM_END





/***************************************************************************

  Coleco Donkey Kong (manufactured in Taiwan, licensed from Nintendo)
  * PCB label Coleco Rev C 75790 DK
  * Hitachi QFP HD38820A45 MCU
  * RC circuit for speaker volume decay
  * cyan/red VFD display Futaba DM-47ZK 2K, with color overlay

***************************************************************************/

class cdkong_state : public hh_hmcs40_state
{
public:
	cdkong_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void cdkong(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);

	void speaker_update();
	TIMER_DEVICE_CALLBACK_MEMBER(speaker_decay_sim);

	double m_speaker_volume = 0.0;
	std::vector<double> m_speaker_levels;
};

void cdkong_state::machine_start()
{
	hh_hmcs40_state::machine_start();
	save_item(NAME(m_speaker_volume));
}

// handlers

void cdkong_state::speaker_update()
{
	if (m_r[1] & 8)
		m_speaker_volume = 1.0;

	int level = (m_d & 8) ? 0x7fff : 0;
	m_speaker->level_w(level * m_speaker_volume);
}

TIMER_DEVICE_CALLBACK_MEMBER(cdkong_state::speaker_decay_sim)
{
	// volume decays when speaker is off (divisor and timer period determine duration)
	speaker_update();
	m_speaker_volume /= 1.02;
}

void cdkong_state::update_display()
{
	u32 plate = bitswap<32>(m_plate,31,30,29,24,0,16,8,1,23,17,9,2,18,10,25,27,26,3,15,27,11,11,14,22,6,13,21,5,19,12,20,4) | 0x800800;
	m_display->matrix(m_grid, plate);
}

void cdkong_state::plate_w(offs_t offset, u8 data)
{
	// R13: speaker on
	m_r[offset] = data;
	speaker_update();

	// R0x-R6x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void cdkong_state::grid_w(u16 data)
{
	// D3: speaker out
	m_d = data;
	speaker_update();

	// D4-D14: vfd grid
	m_grid = data >> 4 & 0x7ff;
	update_display();
}

// config

static INPUT_PORTS_START( cdkong )
	PORT_START("IN.0") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.1") // port D
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x7ff8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void cdkong_state::cdkong(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(cdkong_state::plate_w));
	m_maincpu->write_d().set(FUNC(cdkong_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(605, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(11, 29);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	TIMER(config, "speaker_decay").configure_periodic(FUNC(cdkong_state::speaker_decay_sim), attotime::from_msec(1));

	// set volume levels (set_output_gain is too slow for sub-frame intervals)
	m_speaker_levels.resize(0x8000);
	for (int i = 0; i < 0x8000; i++)
		m_speaker_levels[i] = double(i) / 32768.0;
	m_speaker->set_levels(0x8000, &m_speaker_levels[0]);
}

// roms

ROM_START( cdkong )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a45", 0x0000, 0x1000, CRC(196b8070) SHA1(da85d1eb4b048b77f3168630662ab94ec9baa262) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 359199, "screen", 0)
	ROM_LOAD( "cdkong.svg", 0, 359199, CRC(ba159fd5) SHA1(3188e2ed3234f39ac9ee93a485a7e73314bc3457) )
ROM_END





/***************************************************************************

  Coleco Galaxian (manufactured in Taiwan)
  * PCB label Coleco Rev A 75718
  * Hitachi HD38800A70 MCU
  * discrete sound (when alien attacks)
  * cyan/red VFD display Futaba DM-36Z 2H, with color overlay

  Select game mode on start:
  - P1 Left:  Galaxian (default game)
  - P1 Right: Midway's Attackers
  - P2 Left:  Head-to-Head Galaxian (2-player mode, short)
  - P2 Right: Head-to-Head Galaxian (2-player mode, long)

***************************************************************************/

class cgalaxn_state : public hh_hmcs40_state
{
public:
	cgalaxn_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void cgalaxn(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(player_switch);

private:
	void update_display();
	void grid_w(offs_t offset, u8 data);
	void plate_w(u16 data);
	u8 input_r();
};

// handlers

void cgalaxn_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,1,2,0,11,10,9,8,7,6,5,4,3);
	u16 plate = bitswap<16>(m_plate,15,14,6,5,4,3,2,1,7,8,9,10,11,0,12,13);
	m_display->matrix(grid, plate);
}

INPUT_CHANGED_MEMBER(cgalaxn_state::player_switch)
{
	// 2-player switch directly enables plate 14
	m_plate = (m_plate & 0x3fff) | (newval ? 0 : 0x4000);
	update_display();
}

void cgalaxn_state::grid_w(offs_t offset, u8 data)
{
	// R10,R11: input mux
	if (offset == 1)
		m_inp_mux = data & 3;

	// R1x-R3x: vfd grid
	int shift = (offset - 1) * 4;
	m_grid = (m_grid & ~(0xf << shift)) | (data << shift);
	update_display();
}

void cgalaxn_state::plate_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1: start alien attack whine sound effect (edge triggered)

	// D2-D15: vfd plate
	m_plate = (m_plate & 0x4000) | (data >> 2 & 0x3fff);
	update_display();
}

u8 cgalaxn_state::input_r()
{
	// R0x: multiplexed inputs
	return read_inputs(2);
}

// config

static INPUT_PORTS_START( cgalaxn )
	PORT_START("IN.0") // R10 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // R11 port R0x
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Players ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, cgalaxn_state, player_switch, 0)
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.3") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1)
INPUT_PORTS_END

void cgalaxn_state::cgalaxn(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->read_r<0>().set(FUNC(cgalaxn_state::input_r));
	m_maincpu->write_r<1>().set(FUNC(cgalaxn_state::grid_w));
	m_maincpu->write_r<2>().set(FUNC(cgalaxn_state::grid_w));
	m_maincpu->write_r<3>().set(FUNC(cgalaxn_state::grid_w));
	m_maincpu->write_d().set(FUNC(cgalaxn_state::plate_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(526, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(12, 15);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cgalaxn )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a70", 0x0000, 0x1000, CRC(a4c5ed1d) SHA1(0f647cb78437d7e62411febf7c9ce3c5b6753a80) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 712204, "screen", 0)
	ROM_LOAD( "cgalaxn.svg", 0, 712204, CRC(67ec57bf) SHA1(195c9867b321da9768ce287d1060ceae50345dd4) )
ROM_END





/***************************************************************************

  Coleco Pac-Man (manufactured in Taiwan, licensed from Midway)
  * PCB label Coleco 75690
  * Hitachi QFP HD38820A28/29 MCU
  * cyan/red VFD display Futaba DM-34Z 2A, with color overlay

  known releases:
  - USA: Pac-Man, by Coleco (name-license from Midway)
  - Japan: Super Puck Monster, published by Gakken

  Select game mode on start:
  - P1 Right: Pac-Man (default game)
  - P1 Left:  Head-to-Head Pac-Man (2-player mode)
  - P1 Up:    Eat & Run
  - P1 Down:  Demo

  BTANB: 1st version doesn't show the whole maze on power-on

***************************************************************************/

class cpacman_state : public hh_hmcs40_state
{
public:
	cpacman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void cpacman(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u8 input_r();
};

// handlers

void cpacman_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R6x(,D1,D2): vfd plate
	int shift = (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,0,1,2,3,4,5,6,7,8,9,10);
	u32 plate = bitswap<32>(m_plate,31,30,29,28,27,0,1,2,3,8,9,10,11,16,17,18,19,25,26,23,22,21,20,24,15,14,13,12,4,5,6,7);
	m_display->matrix(grid, plate);
}

void cpacman_state::grid_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D13-D15: input mux
	m_inp_mux = data >> 13 & 7;

	// D5-D15: vfd grid
	m_grid = data >> 5 & 0x7ff;

	// D1,D2: plate 8,14 (update display there)
	plate_w(6 + 1, data >> 1 & 3);
}

u8 cpacman_state::input_r()
{
	// R0x: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( cpacman )
	PORT_START("IN.0") // D13 port R0x
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // D14 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )

	PORT_START("IN.2") // D15 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
INPUT_PORTS_END

void cpacman_state::cpacman(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->read_r<0>().set(FUNC(cpacman_state::input_r));
	m_maincpu->write_r<1>().set(FUNC(cpacman_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(cpacman_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(cpacman_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(cpacman_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(cpacman_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(cpacman_state::plate_w));
	m_maincpu->write_d().set(FUNC(cpacman_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(484, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(11, 27);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cpacman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a29", 0x0000, 0x1000, CRC(1082d577) SHA1(0ef73132bd41f6ca1e4c001ae19f7f7c97eaa8d1) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 359765, "screen", 0)
	ROM_LOAD( "cpacman.svg", 0, 359765, CRC(e3810a46) SHA1(d0994edd71a6adc8f238c71e360a8606ce397a14) )
ROM_END

ROM_START( cpacmanr1 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a28", 0x0000, 0x1000, CRC(d2ed57e5) SHA1(f56f1341485ac28ea9e6cc4d162fab18d8a4c977) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 359765, "screen", 0)
	ROM_LOAD( "cpacman.svg", 0, 359765, CRC(e3810a46) SHA1(d0994edd71a6adc8f238c71e360a8606ce397a14) )
ROM_END





/***************************************************************************

  Coleco Ms. Pac-Man (manufactured in Taiwan, licensed from Midway)
  * PCB label Coleco 911171
  * Hitachi QFP HD38820A61 MCU
  * cyan/red VFD display Futaba DM-60Z 3I, with color overlay

  Select game mode on start:
  - P1 Left:  Ms. Pac-Man (default game)
  - P1 Down:  Head-to-Head Ms. Pac-Man (2-player mode)
  - P1 Up:    Demo

  BTANB: in demo-mode, she hardly ever walks to the upper two rows

***************************************************************************/

class cmspacmn_state : public hh_hmcs40_state
{
public:
	cmspacmn_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void cmspacmn(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u8 input_r();
};

// handlers

void cmspacmn_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R6x(,D0,D1): vfd plate
	int shift = (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 grid = bitswap<16>(m_grid,15,14,13,11,10,9,8,7,6,5,4,3,2,1,0,1);
	u32 plate = bitswap<32>(m_plate,14,13,12,4,5,6,7,24,23,25,22,21,20,13,24,3,19,14,12,11,24,2,10,8,7,25,0,9,1,18,17,16) | 0x1004080;
	m_display->matrix(grid, u64(BIT(m_plate,15)) << 32 | plate);
}

void cmspacmn_state::grid_w(u16 data)
{
	// D2: speaker out
	m_speaker->level_w(data >> 2 & 1);

	// D13-D15: input mux
	m_inp_mux = data >> 13 & 7;

	// D5-D15: vfd grid
	m_grid = data >> 5 & 0x7ff;

	// D0,D1: more plates (update display there)
	plate_w(6 + 1, data & 3);
}

u8 cmspacmn_state::input_r()
{
	// R0x: multiplexed inputs
	return read_inputs(3);
}

// config

static INPUT_PORTS_START( cmspacmn )
	PORT_START("IN.0") // D13 port R0x
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // D14 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_START("IN.2") // D15 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
INPUT_PORTS_END

void cmspacmn_state::cmspacmn(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->read_r<0>().set(FUNC(cmspacmn_state::input_r));
	m_maincpu->write_r<1>().set(FUNC(cmspacmn_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(cmspacmn_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(cmspacmn_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(cmspacmn_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(cmspacmn_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(cmspacmn_state::plate_w));
	m_maincpu->write_d().set(FUNC(cmspacmn_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(481, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(12, 33);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cmspacmn )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a61", 0x0000, 0x1000, CRC(76276318) SHA1(9d6ff3f49b4cdaee5c9e238c1ed638bfb9b99aa7) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 849327, "screen", 0)
	ROM_LOAD( "cmspacmn.svg", 0, 849327, CRC(4110ad07) SHA1(76113a2ce0fb1c6dab4e26fd59a13dc89d950d75) )
ROM_END





/***************************************************************************

  Entex Galaxian 2 (manufactured in Japan)
  * PCB labels ENTEX GALAXIAN PB-118/116/097 80-210137/135/114
  * Hitachi QFP HD38820A13 MCU
  * cyan/red/green VFD display Futaba DM-20

  known releases:
  - USA: Galaxian 2, published by Entex
  - UK: Astro Invader, published by Hales/Entex

***************************************************************************/

class egalaxn2_state : public hh_hmcs40_state
{
public:
	egalaxn2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void egalaxn2(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u8 input_r();
};

// handlers

void egalaxn2_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,15,14,13,12,7,6,5,4,3,2,1,0,19,18,17,16,11,10,9,8);
	m_display->matrix(grid, plate);
}

void egalaxn2_state::grid_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D1-D4: input mux
	m_inp_mux = data >> 1 & 0xf;

	// D1-D15: vfd grid
	m_grid = data >> 1 & 0x7fff;
	update_display();
}

void egalaxn2_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R6x: vfd plate
	int shift = (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

u8 egalaxn2_state::input_r()
{
	// R0x: multiplexed inputs
	return read_inputs(4);
}

// config

static INPUT_PORTS_START( egalaxn2 )
	PORT_START("IN.0") // D1 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.1") // D2 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.2") // D3 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // D4 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_CONFNAME( 0x0c, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "0 (Demo)" ) // for Demo mode: need to hold down Fire button at power-on
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x04, "2" )
INPUT_PORTS_END

void egalaxn2_state::egalaxn2(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->read_r<0>().set(FUNC(egalaxn2_state::input_r));
	m_maincpu->write_r<1>().set(FUNC(egalaxn2_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(egalaxn2_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(egalaxn2_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(egalaxn2_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(egalaxn2_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(egalaxn2_state::plate_w));
	m_maincpu->write_d().set(FUNC(egalaxn2_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(505, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(15, 24);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( egalaxn2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a13", 0x0000, 0x1000, CRC(112b721b) SHA1(4a185bc57ea03fe64f61f7db4da37b16eeb0cb54) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 507945, "screen", 0)
	ROM_LOAD( "egalaxn2.svg", 0, 507945, CRC(b72a8721) SHA1(2d90fca6ce962710525b631e5bc8f75d79332b9d) )
ROM_END





/***************************************************************************

  Entex Pac Man 2 (manufactured in Japan)
  * PCB labels ENTEX PAC-MAN PB-093/094 80-210149/50/51
  * Hitachi QFP HD38820A23 MCU
  * cyan/red VFD display Futaba DM-28Z 1G(cyan Pac-Man) or DM-28 1K(orange Pac-Man)

  2 VFD revisions are known, the difference is Pac-Man's color: cyan or red.

***************************************************************************/

class epacman2_state : public egalaxn2_state
{
public:
	epacman2_state(const machine_config &mconfig, device_type type, const char *tag) :
		egalaxn2_state(mconfig, type, tag)
	{ }

	void epacman2(machine_config &config);
};

// handlers are identical to Galaxian 2, so we can use those

// config

static INPUT_PORTS_START( epacman2 )
	PORT_START("IN.0") // D1 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.1") // D2 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY

	PORT_START("IN.2") // D3 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P1 Skill Control")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Demo Light Test")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // D4 port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFNAME( 0x0c, 0x04, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x08, "0 (Demo)" )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
INPUT_PORTS_END

void epacman2_state::epacman2(machine_config &config)
{
	egalaxn2(config);

	// video hardware
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(505, 1080);
	screen->set_visarea_full();
}

// roms

ROM_START( epacman2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a23", 0x0000, 0x1000, CRC(6eab640f) SHA1(509bdd02be915089e13769f22a08e03509f03af4) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 262480, "screen", 0)
	ROM_LOAD( "epacman2.svg", 0, 262480, CRC(73bd9671) SHA1(a3ac754c0e060da50b65f3d0f9630d9c3d871650) )
ROM_END

ROM_START( epacman2r )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a23", 0x0000, 0x1000, CRC(6eab640f) SHA1(509bdd02be915089e13769f22a08e03509f03af4) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 262483, "screen", 0)
	ROM_LOAD( "epacman2r.svg", 0, 262483, CRC(279b629a) SHA1(4c499fb143aadf4f6722b994a22a0d0d3c5150b6) )
ROM_END





/***************************************************************************

  Entex Super Space Invader 2 (black version)
  * Hitachi HD38800A31 MCU
  * cyan/red VFD display

  This version has the same MCU as the Select-A-Game cartridge. Maybe from
  surplus inventory after that console was discontinued?. It was also sold
  as "Super Alien Invader 2".

  Hold down the fire button at boot for demo mode to work.

***************************************************************************/

class einvader2_state : public hh_hmcs40_state
{
public:
	einvader2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void einvader2(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u16 input_r();
};

// handlers

void einvader2_state::update_display()
{
	m_display->matrix(m_grid, m_plate);
}

void einvader2_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void einvader2_state::grid_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D3,D5,D6: input mux
	m_inp_mux = (data >> 3 & 1) | (data >> 4 & 6);

	// D1-D12: vfd grid
	m_grid = data >> 1 & 0xfff;
	update_display();
}

u16 einvader2_state::input_r()
{
	// D13-D15: multiplexed inputs
	return read_inputs(3) << 13;
}

// config

static INPUT_PORTS_START( einvader2 )
	PORT_START("IN.0") // D3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x01) // 1 player

	PORT_START("IN.1") // D5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, 0x04, IPT_CUSTOM ) PORT_CONDITION("FAKE", 0x03, EQUALS, 0x00) // demo

	PORT_START("IN.2") // D6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("FAKE") // shared D3/D5
	PORT_CONFNAME( 0x03, 0x01, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "Demo" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
INPUT_PORTS_END

void einvader2_state::einvader2(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 450000); // approximation
	m_maincpu->write_r<0>().set(FUNC(einvader2_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(einvader2_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(einvader2_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(einvader2_state::plate_w));
	m_maincpu->write_d().set(FUNC(einvader2_state::grid_w));
	m_maincpu->read_d().set(FUNC(einvader2_state::input_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(469, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(12, 14);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( einvader2 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "inv2_hd38800a31", 0x0000, 0x1000, CRC(10e39521) SHA1(41d86696e518ea071e75ed37d5dc63c0408c262e) )
	ROM_CONTINUE(                0x1e80, 0x0100 )

	ROM_REGION( 217430, "screen", 0)
	ROM_LOAD( "einvader2.svg", 0, 217430, CRC(b082d9a3) SHA1(67f7e0314c69ba146751ea12cf490805b8660489) )
ROM_END





/***************************************************************************

  Entex Turtles (manufactured in Japan)
  * PCB label 560359
  * Hitachi QFP HD38820A43 MCU
  * COP411L sub MCU, label COP411L-KED/N
  * cyan/red/green VFD display NEC FIP15BM32T

***************************************************************************/

class eturtles_state : public hh_hmcs40_state
{
public:
	eturtles_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{ }

	void eturtles(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int(); }

protected:
	virtual void machine_start() override;

	required_device<cop411_cpu_device> m_audiocpu;

	void update_int();
	virtual void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);

	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	void cop_irq_w(u8 data);
	u8 cop_latch_r();
	u8 cop_ack_r();

	u8 m_cop_irq = 0;
};

void eturtles_state::machine_start()
{
	hh_hmcs40_state::machine_start();
	save_item(NAME(m_cop_irq));
}

// handlers: maincpu side

void eturtles_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,1,14,13,12,11,10,9,8,7,6,5,4,3,2,0);
	u32 plate = bitswap<32>(m_plate,31,30,11,12,18,19,16,17,22,15,20,21,27,26,23,25,24,2,3,1,0,6,4,5,10,9,2,8,7,14,1,13);
	m_display->matrix(grid, plate | (grid >> 5 & 8)); // grid 8 also forces plate 3 high
}

void eturtles_state::plate_w(offs_t offset, u8 data)
{
	m_r[offset] = data;

	// R0x-R6x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void eturtles_state::grid_w(u16 data)
{
	m_d = data;

	// D1-D6: input mux
	u8 inp_mux = data >> 1 & 0x3f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int();
	}

	// D1-D15: vfd grid
	m_grid = data >> 1 & 0x7fff;
	update_display();
}

void eturtles_state::update_int()
{
	// INT0/1 on multiplexed inputs, and from COP D0
	u8 inp = read_inputs(6);
	set_interrupt(0, (inp & 1) | m_cop_irq);
	set_interrupt(1, inp & 2);
}

// handlers: COP side

WRITE_LINE_MEMBER(eturtles_state::speaker_w)
{
	// SK: speaker out
	m_speaker->level_w(!state);
}

void eturtles_state::cop_irq_w(u8 data)
{
	// D0: maincpu INT0 (active low)
	m_cop_irq = ~data & 1;
	update_int();
}

u8 eturtles_state::cop_latch_r()
{
	// L0-L3: soundlatch from maincpu R0x
	return m_r[0];
}

u8 eturtles_state::cop_ack_r()
{
	// G0: ack from maincpu D0
	return m_d & 1;
}

// config

static INPUT_PORTS_START( eturtles )
	PORT_START("IN.0") // D1 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)

	PORT_START("IN.1") // D2 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)

	PORT_START("IN.2") // D3 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)

	PORT_START("IN.3") // D4 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)

	PORT_START("IN.4") // D5 INT0/1
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)

	PORT_START("IN.5") // D6 INT0/1
	PORT_CONFNAME( 0x03, 0x00, DEF_STR( Players ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_CONFSETTING(    0x02, "0 (Demo)" )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
INPUT_PORTS_END

void eturtles_state::eturtles(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(eturtles_state::plate_w));
	m_maincpu->write_d().set(FUNC(eturtles_state::grid_w));

	COP411(config, m_audiocpu, 215000); // approximation
	m_audiocpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_audiocpu->write_sk().set(FUNC(eturtles_state::speaker_w));
	m_audiocpu->write_d().set(FUNC(eturtles_state::cop_irq_w));
	m_audiocpu->read_l().set(FUNC(eturtles_state::cop_latch_r));
	m_audiocpu->read_g().set(FUNC(eturtles_state::cop_ack_r));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(484, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(15, 30);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( eturtles )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a43", 0x0000, 0x1000, CRC(446aa4e2) SHA1(d1c0fb14ea7081def53b1174964b39eed1e5d5e6) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 0x0200, "audiocpu", 0 )
	ROM_LOAD( "cop411l-ked_n", 0x0000, 0x0200, CRC(503d26e9) SHA1(a53d24d62195bfbceff2e4a43199846e0950aef6) )

	ROM_REGION( 1027626, "screen", 0)
	ROM_LOAD( "eturtles.svg", 0, 1027626, CRC(b4f7abff) SHA1(e9b065a3a3fef3c71495002945724a86c2a68eb4) )
ROM_END





/***************************************************************************

  Entex Stargate (manufactured in Japan)
  * PCB label 5603521/31
  * Hitachi QFP HD38820A42 MCU
  * COP411L sub MCU, label ~/B8236 COP411L-KEC/N
  * cyan/red/green VFD display NEC FIP15AM32T (EL628-003) no. 2-421, with partial color overlay

***************************************************************************/

class estargte_state : public eturtles_state
{
public:
	estargte_state(const machine_config &mconfig, device_type type, const char *tag) :
		eturtles_state(mconfig, type, tag)
	{ }

	void estargte(machine_config &config);

private:
	virtual void update_display() override;
	u8 cop_data_r();
};

// handlers (most of it is in eturtles_state above)

void estargte_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,0,14,13,12,11,10,9,8,7,6,5,4,3,2,1);
	u32 plate = bitswap<32>(m_plate,31,30,29,15,17,19,21,23,25,27,26,24,3,22,20,18,16,14,12,10,8,6,4,2,0,1,3,5,7,9,11,13);
	m_display->matrix(grid, plate);
}

u8 estargte_state::cop_data_r()
{
	// L0-L3: soundlatch from maincpu R0x
	// L7: ack from maincpu D0
	return m_r[0] | (m_d << 7 & 0x80);
}

// config

static INPUT_PORTS_START( estargte )
	PORT_START("IN.0") // D1 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0) PORT_NAME("Inviso")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0) PORT_NAME("Smart Bomb")

	PORT_START("IN.1") // D2 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0) PORT_NAME("Change Direction")

	PORT_START("IN.2") // D3 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)

	PORT_START("IN.3") // D4 INT0/1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0) PORT_NAME("Thrust")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // D5 INT0/1
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Players ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_CONFSETTING(    0x00, "0 (Demo)" ) // yes, same value as 1-player, hold the Inviso button at boot to enter demo mode
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFNAME( 0x02, 0x02, DEF_STR( Difficulty ) ) PORT_CHANGED_MEMBER(DEVICE_SELF, eturtles_state, input_changed, 0)
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x02, "2" )

	PORT_START("IN.5") // D6 INT0/1
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void estargte_state::estargte(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(estargte_state::plate_w));
	m_maincpu->write_d().set(FUNC(estargte_state::grid_w));

	COP411(config, m_audiocpu, 190000); // approximation
	m_audiocpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false); // guessed
	m_audiocpu->write_sk().set(FUNC(estargte_state::speaker_w));
	m_audiocpu->write_d().set(FUNC(estargte_state::cop_irq_w));
	m_audiocpu->read_l().set(FUNC(estargte_state::cop_data_r));

	config.set_perfect_quantum(m_maincpu);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 854);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(14, 29);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( estargte )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a42", 0x0000, 0x1000, CRC(5f6d55a6) SHA1(0da32149790fa5f16097338fc80536b462169e0c) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 0x0200, "audiocpu", 0 )
	ROM_LOAD( "cop411l-kec_n", 0x0000, 0x0200, CRC(fbd3c2d3) SHA1(65b8b24d38678c3fa970bfd639e9449a75a28927) )

	ROM_REGION( 462214, "screen", 0)
	ROM_LOAD( "estargte.svg", 0, 462214, CRC(282cc090) SHA1(b0f3c21e9a529e5f1e33b90ca25ce3a097fb75a0) )
ROM_END





/***************************************************************************

  Gakken Heiankyo Alien (manufactured in Japan)
  * Hitachi HD38800A04 MCU
  * cyan/red VFD display Futaba DM-11Z 1H

  known releases:
  - Japan: Heiankyo Alien, published by Gakken
  - USA: Earth Invaders, published by CGL

***************************************************************************/

class ghalien_state : public hh_hmcs40_state
{
public:
	ghalien_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void ghalien(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	u16 input_r();
};

// handlers

void ghalien_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D10-D13): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,14,12,10,8,9,13,15,2,0,1,3,11,7,5,4,6,19,17,16,18);
	m_display->matrix(grid, plate);
}

void ghalien_state::grid_w(u16 data)
{
	// D14: speaker out
	m_speaker->level_w(data >> 14 & 1);

	// D0-D6: input mux
	m_inp_mux = data & 0x7f;

	// D0-D9: vfd grid
	m_grid = data & 0x3ff;

	// D10-D13: more plates (update display there)
	plate_w(4, data >> 10 & 0xf);
}

u16 ghalien_state::input_r()
{
	// D15: multiplexed inputs
	return read_inputs(7) & 0x8000;
}

// config

static INPUT_PORTS_START( ghalien )
	PORT_START("IN.0") // D0 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY

	PORT_START("IN.1") // D1 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY

	PORT_START("IN.2") // D2 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY

	PORT_START("IN.3") // D3 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY

	PORT_START("IN.4") // D4 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Dig")

	PORT_START("IN.5") // D5 line D15
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bury")

	PORT_START("IN.6") // D6 line D15
	PORT_CONFNAME( 0x8000, 0x0000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x0000, "Amateur" )
	PORT_CONFSETTING(      0x8000, "Professional" )
INPUT_PORTS_END

void ghalien_state::ghalien(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(ghalien_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(ghalien_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(ghalien_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(ghalien_state::plate_w));
	m_maincpu->write_d().set(FUNC(ghalien_state::grid_w));
	m_maincpu->read_d().set(FUNC(ghalien_state::input_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 699);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 20);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( ghalien )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a04", 0x0000, 0x1000, CRC(019c3328) SHA1(9f1029c5c479f78350952c4f18747341ba5ea7a0) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 462749, "screen", 0)
	ROM_LOAD( "ghalien.svg", 0, 462749, CRC(1acbb1e8) SHA1(7bdeb840bc9080792e24812eba923bf84f7865a6) )
ROM_END





/***************************************************************************

  Gakken Crazy Kong (manufactured in Japan)
  * PCB label ZENY 5603601
  * Hitachi HD38800B01 MCU
  * cyan/red/blue VFD display Futaba DM-54Z 2H, with bezel overlay

  known releases:
  - Japan: Crazy Kong, published by Gakken
  - USA: Super Kong, published by CGL

***************************************************************************/

class gckong_state : public hh_hmcs40_state
{
public:
	gckong_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void gckong(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int1(); }

private:
	void update_int1();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void gckong_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x(,D0,D1): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,0,1,2,3,4,5,6,7,8,9,10);
	u32 plate = bitswap<32>(m_plate,31,30,29,28,27,26,25,6,7,8,12,13,14,15,16,17,18,17,16,12,11,10,9,8,7,6,5,4,3,2,1,0) | 0x8000;
	m_display->matrix(grid, plate);
}

void gckong_state::grid_w(u16 data)
{
	// D2: speaker out
	m_speaker->level_w(data >> 2 & 1);

	// D5-D8: input mux
	u8 inp_mux = data >> 5 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D5-D15: vfd grid
	m_grid = data >> 5 & 0x7ff;

	// D0,D1: more plates (update display there)
	plate_w(4, data & 3);
}

void gckong_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(4));
}

// config

static INPUT_PORTS_START( gckong )
	PORT_START("IN.0") // D5 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, 0)

	PORT_START("IN.1") // D6 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, 0)

	PORT_START("IN.2") // D7 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, 0)

	PORT_START("IN.3") // D8 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, gckong_state, input_changed, 0)

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)

	PORT_START("IN.5") // port D
	PORT_CONFNAME( 0x0010, 0x0000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x0000, "A" )
	PORT_CONFSETTING(      0x0010, "B" )
	PORT_BIT( 0xffef, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void gckong_state::gckong(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(gckong_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(gckong_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(gckong_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(gckong_state::plate_w));
	m_maincpu->write_d().set(FUNC(gckong_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.5");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(479, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(11, 32);
	config.set_default_layout(layout_gckong);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gckong )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b01", 0x0000, 0x1000, CRC(d5a2cca3) SHA1(37bb5784383daab672ed1e0e2362c7a40d8d9b3f) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 346588, "screen", 0)
	ROM_LOAD( "gckong.svg", 0, 346588, CRC(317af984) SHA1(ff6323526d1f5e46eccf8fa8d979175895be75de) )
ROM_END





/***************************************************************************

  Gakken Super Cobra
  * PCB label SUPER COBRA 3000N
  * Hitachi QFP HD38820A32 MCU
  * cyan/red/green VFD display

  known releases:
  - World: Super Cobra, published by Gakken
  - USA: Cobra Super Copter, published by Tandy

  There are 2 versions, a green one and a white one. They have the same MCU,
  though the VFD has color differences and is more compact.

  BTANB(green version): 1 rocket seems out of place at the top-right area

***************************************************************************/

class gscobra_state : public hh_hmcs40_state
{
public:
	gscobra_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void gscobra(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int0(); }

private:
	void update_int0();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void gscobra_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R6x(,D1-D3): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	m_display->matrix(m_grid, m_plate);
}

void gscobra_state::grid_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D10-D15: input mux
	u8 inp_mux = data >> 10 & 0x3f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D7-D15: vfd grid
	m_grid = data >> 7 & 0x1ff;

	// D1-D3: more plates (update display there)
	plate_w(7, data >> 1 & 7);
}

void gscobra_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(6));
}

// config

static INPUT_PORTS_START( gscobra )
	PORT_START("IN.0") // D10 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gscobra_state, input_changed, 0)

	PORT_START("IN.1") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, gscobra_state, input_changed, 0)

	PORT_START("IN.2") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, gscobra_state, input_changed, 0)

	PORT_START("IN.3") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, gscobra_state, input_changed, 0)

	PORT_START("IN.4") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gscobra_state, input_changed, 0)

	PORT_START("IN.5") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gscobra_state, input_changed, 0)
INPUT_PORTS_END

void gscobra_state::gscobra(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(gscobra_state::plate_w));
	m_maincpu->write_d().set(FUNC(gscobra_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 852);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 31);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gscobra )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a32", 0x0000, 0x1000, CRC(7bbd130f) SHA1(91dd280e4108fad7ba99191355364bd3217b9d17) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 232919, "screen", 0)
	ROM_LOAD( "gscobra.svg", 0, 232919, CRC(5ceb4bfc) SHA1(77c9a45569d780838ebe75818acb2d2ced4bda00) )
ROM_END





/***************************************************************************

  Gakken Dig Dug (manufactured in Japan)
  * PCB label Gakken DIG-DAG KS-004283(A/B)
  * Hitachi QFP HD38820A69 MCU
  * cyan/red/green VFD display Futaba DM-69Z 3F, with color overlay

***************************************************************************/

class gdigdug_state : public hh_hmcs40_state
{
public:
	gdigdug_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void gdigdug(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int1(); }

private:
	void update_int1();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void gdigdug_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R6x(,D0-D3): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u32 plate = bitswap<32>(m_plate,30,31,0,1,2,3,4,5,6,7,20,21,22,27,26,25,28,29,24,23,15,14,13,12,8,9,10,11,19,18,17,16);
	m_display->matrix(m_grid, plate);
}

void gdigdug_state::grid_w(u16 data)
{
	// D6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// D11-D15: input mux
	u8 inp_mux = data >> 11 & 0x1f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D7-D15: vfd grid
	m_grid = data >> 7 & 0x1ff;

	// D0-D3: more plates (update display there)
	plate_w(7, data & 0xf);
}

void gdigdug_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(5));
}

// config

static INPUT_PORTS_START( gdigdug )
	PORT_START("IN.0") // D11 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, 0)

	PORT_START("IN.1") // D12 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, 0)

	PORT_START("IN.2") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, 0)

	PORT_START("IN.3") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, 0)

	PORT_START("IN.4") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, gdigdug_state, input_changed, 0)

	PORT_START("IN.5") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)
INPUT_PORTS_END

void gdigdug_state::gdigdug(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(gdigdug_state::plate_w));
	m_maincpu->write_d().set(FUNC(gdigdug_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(476, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gdigdug )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a69", 0x0000, 0x1000, CRC(501165a9) SHA1(8a15d00c4aa66e870cadde33148426463560d2e6) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 807990, "screen", 0)
	ROM_LOAD( "gdigdug.svg", 0, 807990, CRC(a5b8392d) SHA1(3503829bb1a626a9e70115fb60b656dff8908144) )
ROM_END





/***************************************************************************

  Mattel World Championship Baseball (model 3201)
  * PCB label MEL-001 Baseball Rev. B
  * Hitachi QFP HD38820A09 MCU
  * cyan/red/green VFD display Futaba DM-24ZK 1G, with etched overlay

  It was patented under US4372557. To start the game in 2-player mode, simply
  turn the game on. For 1-player, turn the game on while holding the 1-key
  and use the visitor's side keypad to play offsense.

***************************************************************************/

class mwcbaseb_state : public hh_hmcs40_state
{
public:
	mwcbaseb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void mwcbaseb(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	void speaker_w(u8 data);
	u8 input_r();
};

// handlers

void mwcbaseb_state::update_display()
{
	u8 grid = bitswap<8>(m_grid,0,1,2,3,4,5,6,7);
	m_display->matrix(grid, m_plate);
}

void mwcbaseb_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R3x,R6x: vfd plate
	int shift = (offset == 6) ? 12 : (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void mwcbaseb_state::grid_w(u16 data)
{
	// D9-D15: input mux
	m_inp_mux = data >> 9 & 0x7f;

	// D0-D7: vfd grid
	m_grid = data & 0xff;
	update_display();
}

void mwcbaseb_state::speaker_w(u8 data)
{
	// R50,R51+R52(tied together): speaker out
	m_speaker->level_w(data & 7);
}

u8 mwcbaseb_state::input_r()
{
	// R4x: multiplexed inputs
	return read_inputs(7);
}

// config

/* physical button layout and labels are like this:

        (visitor team side)                                       (home team side)
    COMP PITCH                     [SCORE]       [INNING]
    [1]      [2]      [3]                                     [1]      [2]      [3]
    NEW PITCHER       PINCH HITTER                            NEW PITCHER       PINCH HITTER

    [4]      [5]      [6]                                     [4]      [5]      [6]
    BACKWARD (pitch)  FORWARD                                 BACKWARD (pitch)  FORWARD

    [7]      [8]      [9]                                     [7]      [8]      [9]

    BUNT     NORMAL   HR SWING                                BUNT     NORMAL   HR SWING
    [CLEAR]  [0]      [ENTER]                                 [CLEAR]  [0]      [ENTER]
    SLOW     CURVE    FAST                                    SLOW     CURVE    FAST
*/

static INPUT_PORTS_START( mwcbaseb ) // P1 = left/visitor, P2 = right/home
	PORT_START("IN.0") // D9 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("P2 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("P2 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("P2 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("P2 1")

	PORT_START("IN.1") // D10 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P2 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P2 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("P2 5")

	PORT_START("IN.2") // D11 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P2 Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("P2 Clear")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("P2 0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("P2 9")

	PORT_START("IN.3") // D12 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Inning")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Score")

	PORT_START("IN.4") // D13 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P1 Enter")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("P1 Clear")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("P1 0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("P1 9")

	PORT_START("IN.5") // D14 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("P1 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("P1 7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("P1 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("P1 5")

	PORT_START("IN.6") // D15 port R4x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("P1 4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("P1 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("P1 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("P1 1")
INPUT_PORTS_END

void mwcbaseb_state::mwcbaseb(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<1>().set(FUNC(mwcbaseb_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(mwcbaseb_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(mwcbaseb_state::plate_w));
	m_maincpu->read_r<4>().set(FUNC(mwcbaseb_state::input_r));
	m_maincpu->write_r<5>().set(FUNC(mwcbaseb_state::speaker_w));
	m_maincpu->write_r<6>().set(FUNC(mwcbaseb_state::plate_w));
	m_maincpu->write_d().set(FUNC(mwcbaseb_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 478);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 16);
	m_display->set_bri_levels(0.001); // cyan elements strobed very briefly?
	config.set_default_layout(layout_mwcbaseb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
	static const double speaker_levels[] = { 0.0, 0.5, -0.5, 0.0, -0.5, 0.0, -1.0, -0.5 };
	m_speaker->set_levels(8, speaker_levels);
}

// roms

ROM_START( mwcbaseb )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a09", 0x0000, 0x1000, CRC(25ba7dc0) SHA1(69e0a867fdcf07b454b1faf835e576ae782432c0) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 178441, "screen", 0)
	ROM_LOAD( "mwcbaseb.svg", 0, 178441, CRC(0f631190) SHA1(74a10ad0630af5516f76d5bf5628483d21f6b7be) )
ROM_END





/***************************************************************************

  Mattel Star Hawk (manufactured in Japan)
  * PCB label Kaken, PT-317B
  * Hitachi HD38800A73 MCU
  * cyan/red VFD display Futaba DM-41ZK, with partial color overlay + bezel

  Before release, it was advertised as "Space Battle"(a Mattel Intellivision game).
  Kaken was a subsidiary of Bandai. Star Hawk shell design is the same as Bandai's
  games from the same era. It's likely that this was made under contract exclusively
  for Mattel. There is no indication that this game was released in Japan by Bandai.

***************************************************************************/

class msthawk_state : public hh_hmcs40_state
{
public:
	msthawk_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void msthawk(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int0(); }

private:
	void update_int0();
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void msthawk_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,0,1,2,3,4,5,6,7,8,9);
	u32 plate = bitswap<24>(m_plate,23,22,21,19,20,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
	m_display->matrix(grid, plate);
}

void msthawk_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void msthawk_state::grid_w(u16 data)
{
	// D5: speaker out
	m_speaker->level_w(data >> 5 & 1);

	// D10-D15: input mux
	u8 inp_mux = data >> 10 & 0x3f;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D6-D15: vfd grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D4: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x1f0000);
	update_display();
}

void msthawk_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(6));
}

// config

static INPUT_PORTS_START( msthawk )
	PORT_START("IN.0") // D10 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, 0) PORT_NAME("Score")

	PORT_START("IN.1") // D11 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, 0) PORT_NAME("Land")

	PORT_START("IN.2") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, 0)

	PORT_START("IN.3") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, 0)

	PORT_START("IN.4") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, 0)

	PORT_START("IN.5") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, msthawk_state, input_changed, 0)

	PORT_START("IN.6") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1) PORT_NAME("Fire")
INPUT_PORTS_END

void msthawk_state::msthawk(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(msthawk_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(msthawk_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(msthawk_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(msthawk_state::plate_w));
	m_maincpu->write_d().set(FUNC(msthawk_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 696);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 21);
	config.set_default_layout(layout_msthawk);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( msthawk )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a73", 0x0000, 0x1000, CRC(a4f9a523) SHA1(465f06b02e2e7d2277218fd447830725790a816c) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 191888, "screen", 0)
	ROM_LOAD( "msthawk.svg", 0, 191888, CRC(a607fc0f) SHA1(282a412f6462128e09ee8bd18d682dda01297611) )
ROM_END





/***************************************************************************

  Parker Brothers Q*Bert
  * PCB label 13662 REV-4
  * Hitachi QFP HD38820A70 MCU
  * cyan/red/green/darkgreen VFD display Itron CP5137

***************************************************************************/

class pbqbert_state : public hh_hmcs40_state
{
public:
	pbqbert_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void pbqbert(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void pbqbert_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R6x(,D8): vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u32 plate = bitswap<32>(m_plate,31,30,24,25,26,27,28,15,14,29,13,12,11,10,9,8,7,6,5,4,3,2,1,0,16,17,18,19,20,21,22,23) | 0x400000;
	m_display->matrix(m_grid, plate);
}

void pbqbert_state::grid_w(u16 data)
{
	// D14: speaker out
	m_speaker->level_w(data >> 14 & 1);

	// D0-D7: vfd grid
	m_grid = data & 0xff;

	// D8: plate 25 (update display there)
	plate_w(7, data >> 8 & 1);
}

// config

static INPUT_PORTS_START( pbqbert )
	PORT_START("IN.0") // port D
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) // up-left
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) // up-right
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) // down-right
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) // down-left
	PORT_BIT( 0xe1ff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void pbqbert_state::pbqbert(machine_config &config)
{
	// basic machine hardware
	HD38820(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_r<4>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_r<5>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_r<6>().set(FUNC(pbqbert_state::plate_w));
	m_maincpu->write_d().set(FUNC(pbqbert_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.0");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(603, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(8, 30);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( pbqbert )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38820a70", 0x0000, 0x1000, CRC(be7c80b4) SHA1(0617a80ef7fe188ea221de32e760d45fd4318c67) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 456567, "screen", 0)
	ROM_LOAD( "pbqbert.svg", 0, 456567, CRC(49853a62) SHA1(869377109fb7163e5ef5efadb26ce3955231f6ca) )
ROM_END





/***************************************************************************

  Tomy(tronic) Tron (manufactured in Japan)
  * PCB label THN-02 2E114E07
  * Hitachi HD38800A88 MCU
  * cyan/red/green VFD display NEC FIP10AM24T no. 2-8 1

***************************************************************************/

class tmtron_state : public hh_hmcs40_state
{
public:
	tmtron_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void tmtron(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int1(); }

private:
	void update_int1();
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void tmtron_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,1,2,3,4,5,6,7,8,9,0);
	u32 plate = bitswap<24>(m_plate,23,5,2,21,1,6,7,9,10,11,21,0,19,3,4,8,3,18,17,16,12,13,14,15);
	m_display->matrix(grid, plate);
}

void tmtron_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void tmtron_state::grid_w(u16 data)
{
	// D4: speaker out
	m_speaker->level_w(data >> 4 & 1);

	// D12-D15: input mux
	u8 inp_mux = data >> 12 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int1();
	}

	// D6-D15: vfd grid
	m_grid = data >> 6 & 0x3ff;

	// D0-D3,D5: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x2f0000);
	update_display();
}

void tmtron_state::update_int1()
{
	// INT1 on multiplexed inputs
	set_interrupt(1, read_inputs(4));
}

// config

static INPUT_PORTS_START( tmtron )
	PORT_START("IN.0") // D12 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, 0)

	PORT_START("IN.1") // D13 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, 0)

	PORT_START("IN.2") // D14 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, 0)

	PORT_START("IN.3") // D15 INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_CHANGED_MEMBER(DEVICE_SELF, tmtron_state, input_changed, 0)

	PORT_START("IN.4") // INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 0)
INPUT_PORTS_END

void tmtron_state::tmtron(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(tmtron_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(tmtron_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(tmtron_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(tmtron_state::plate_w));
	m_maincpu->write_d().set(FUNC(tmtron_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920, 662);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(10, 23);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tmtron )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800a88", 0x0000, 0x1000, CRC(33db9670) SHA1(d6f747a59356526698784047bcfdbb59e79b9a23) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 384174, "screen", 0)
	ROM_LOAD( "tmtron.svg", 0, 384174, CRC(06bd9e63) SHA1(fb93013ec42dc05f7029ef3c3073c84867f0d077) )
ROM_END





/***************************************************************************

  Tomy Kingman (manufactured in Japan)
  * PCB label THF-01II 2E138E01/2E128E02
  * Hitachi HD38800B23 MCU
  * cyan/red/blue VFD display Futaba DM-65ZK 3A

  known releases:
  - World: Kingman, published by Tomy
  - USA: Kingman, published by Tandy

***************************************************************************/

class kingman_state : public hh_hmcs40_state
{
public:
	kingman_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void kingman(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed) { update_int0(); }

private:
	void update_int0();
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void kingman_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,0,1,2,3,4,5,6,7,8);
	u32 plate = bitswap<24>(m_plate,23,6,7,5,4,3,2,1,0,13,12,20,19,18,17,16,10,11,9,8,14,15,13,12);
	m_display->matrix(grid, plate);
}

void kingman_state::plate_w(offs_t offset, u8 data)
{
	// R0x-R3x: vfd plate
	int shift = offset * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);
	update_display();
}

void kingman_state::grid_w(u16 data)
{
	// D6: speaker out
	m_speaker->level_w(data >> 6 & 1);

	// D12-D15: input mux
	u8 inp_mux = data >> 12 & 0xf;
	if (inp_mux != m_inp_mux)
	{
		m_inp_mux = inp_mux;
		update_int0();
	}

	// D7-D15: vfd grid
	m_grid = data >> 7 & 0x1ff;

	// D0-D4: more plates
	m_plate = (m_plate & 0x00ffff) | (data << 16 & 0x1f0000);
	update_display();
}

void kingman_state::update_int0()
{
	// INT0 on multiplexed inputs
	set_interrupt(0, read_inputs(4));
}

// config

static INPUT_PORTS_START( kingman )
	PORT_START("IN.0") // D12 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, 0)

	PORT_START("IN.1") // D13 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, 0)

	PORT_START("IN.2") // D14 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, 0)

	PORT_START("IN.3") // D15 INT0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, kingman_state, input_changed, 0)

	PORT_START("IN.4") // INT1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_hmcs40_state, single_interrupt_line, 1)
INPUT_PORTS_END

void kingman_state::kingman(machine_config &config)
{
	// basic machine hardware
	HD38800(config, m_maincpu, 400000); // approximation
	m_maincpu->write_r<0>().set(FUNC(kingman_state::plate_w));
	m_maincpu->write_r<1>().set(FUNC(kingman_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(kingman_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(kingman_state::plate_w));
	m_maincpu->write_d().set(FUNC(kingman_state::grid_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(374, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 23);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( kingman )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38800b23", 0x0000, 0x1000, CRC(f8dfe14f) SHA1(660610d92ae7e5f92bddf5a3bcc2296b2ec3946b) )
	ROM_CONTINUE(           0x1e80, 0x0100 )

	ROM_REGION( 396320, "screen", 0)
	ROM_LOAD( "kingman.svg", 0, 396320, CRC(3f52d2a9) SHA1(9291f1a1da3d19c3d6dedb995de0a5feba75b442) )
ROM_END





/***************************************************************************

  VTech Invaders (manufactured in Taiwan)
  * Hitachi HD38750A45 MCU
  * cyan/red VFD display Futaba DM-26Z 1G, with bezel

  known releases:
  - USA: Invaders/Sonic Invader, published by VTech
  - UK: Cosmic Invader, published by Grandstand
  - UK: Galactic Invaders, published by Prinztronic

***************************************************************************/

class vinvader_state : public hh_hmcs40_state
{
public:
	vinvader_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_hmcs40_state(mconfig, type, tag)
	{ }

	void vinvader(machine_config &config);

private:
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
};

// handlers

void vinvader_state::plate_w(offs_t offset, u8 data)
{
	// R1x-R3x(,D4-D6): vfd plate
	int shift = (offset - 1) * 4;
	m_plate = (m_plate & ~(0xf << shift)) | (data << shift);

	// update display
	u16 plate = bitswap<16>(m_plate,15,11,7,3,10,6,14,2,9,5,13,1,8,4,12,0);
	m_display->matrix(m_grid, plate);
}

void vinvader_state::grid_w(u16 data)
{
	// D0: speaker out
	m_speaker->level_w(data & 1);

	// D7-D15: vfd grid
	m_grid = data >> 7 & 0x1ff;

	// D4-D6: more plates (update display there)
	plate_w(3 + 1, data >> 4 & 7);
}

// config

static INPUT_PORTS_START( vinvader )
	PORT_START("IN.0") // port R0x
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // port D
	PORT_CONFNAME( 0x0002, 0x0000, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x0000, "1" )
	PORT_CONFSETTING(      0x0002, "2" )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xfff5, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void vinvader_state::vinvader(machine_config &config)
{
	// basic machine hardware
	HD38750(config, m_maincpu, 300000); // approximation
	m_maincpu->read_r<0>().set_ioport("IN.0");
	m_maincpu->write_r<1>().set(FUNC(vinvader_state::plate_w));
	m_maincpu->write_r<2>().set(FUNC(vinvader_state::plate_w));
	m_maincpu->write_r<3>().set(FUNC(vinvader_state::plate_w));
	m_maincpu->write_d().set(FUNC(vinvader_state::grid_w));
	m_maincpu->read_d().set_ioport("IN.1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(233, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(9, 12);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( vinvader )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd38750a45", 0x0000, 0x0800, CRC(32de6056) SHA1(70238c6c40c3d513f8eced1cb81bdd4dbe12f16c) )
	ROM_CONTINUE(           0x0f00, 0x0080 )

	ROM_REGION( 166379, "screen", 0)
	ROM_LOAD( "vinvader.svg", 0, 166379, CRC(b75c448e) SHA1(40d546f9fbdb446883e3ab0e3f678f1be8105159) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT   CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, bambball,  0,        0, bambball, bambball, bambball_state, empty_init, "Bambino", "Dribble Away Basketball", MACHINE_SUPPORTS_SAVE )
CONS( 1979, bmboxing,  0,        0, bmboxing, bmboxing, bmboxing_state, empty_init, "Bambino", "Knock-Em Out Boxing", MACHINE_SUPPORTS_SAVE )

CONS( 1982, bfriskyt,  0,        0, bfriskyt, bfriskyt, bfriskyt_state, empty_init, "Bandai", "Frisky Tom (Bandai)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, packmon,   0,        0, packmon,  packmon,  packmon_state,  empty_init, "Bandai", "Packri Monster", MACHINE_SUPPORTS_SAVE )
CONS( 1982, bzaxxon,   0,        0, bzaxxon,  bzaxxon,  bzaxxon_state,  empty_init, "Bandai", "Zaxxon (Bandai)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, zackman,   0,        0, zackman,  zackman,  zackman_state,  empty_init, "Bandai", "Zackman", MACHINE_SUPPORTS_SAVE )
CONS( 1983, bpengo,    0,        0, bpengo,   bpengo,   bpengo_state,   empty_init, "Bandai", "Pengo (Bandai)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, bbtime,    0,        0, bbtime,   bbtime,   bbtime_state,   empty_init, "Bandai", "Burger Time (Bandai)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, bdoramon,  0,        0, bdoramon, bdoramon, bdoramon_state, empty_init, "Bandai", "Dokodemo Dorayaki Doraemon", MACHINE_SUPPORTS_SAVE )
CONS( 1983, bultrman,  0,        0, bultrman, bultrman, bultrman_state, empty_init, "Bandai", "Ultraman Monster Battle", MACHINE_SUPPORTS_SAVE )
CONS( 1984, machiman,  0,        0, machiman, machiman, machiman_state, empty_init, "Bandai", "Machine Man", MACHINE_SUPPORTS_SAVE )
CONS( 1984, pairmtch,  0,        0, pairmtch, pairmtch, pairmtch_state, empty_init, "Bandai", "Pair Match", MACHINE_SUPPORTS_SAVE )

CONS( 1981, alnattck,  0,        0, alnattck, alnattck, alnattck_state, empty_init, "Coleco", "Alien Attack", MACHINE_SUPPORTS_SAVE )
CONS( 1982, cdkong,    0,        0, cdkong,   cdkong,   cdkong_state,   empty_init, "Coleco", "Donkey Kong (Coleco)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, cgalaxn,   0,        0, cgalaxn,  cgalaxn,  cgalaxn_state,  empty_init, "Coleco", "Galaxian (Coleco)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
CONS( 1981, cpacman,   0,        0, cpacman,  cpacman,  cpacman_state,  empty_init, "Coleco", "Pac-Man (Coleco, Rev. 29)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, cpacmanr1, cpacman,  0, cpacman,  cpacman,  cpacman_state,  empty_init, "Coleco", "Pac-Man (Coleco, Rev. 28)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, cmspacmn,  0,        0, cmspacmn, cmspacmn, cmspacmn_state, empty_init, "Coleco", "Ms. Pac-Man (Coleco)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, egalaxn2,  0,        0, egalaxn2, egalaxn2, egalaxn2_state, empty_init, "Entex", "Galaxian 2 (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, epacman2,  0,        0, epacman2, epacman2, epacman2_state, empty_init, "Entex", "Pac Man 2 (Entex, cyan Pacman)", MACHINE_SUPPORTS_SAVE )
CONS( 1981, epacman2r, epacman2, 0, epacman2, epacman2, epacman2_state, empty_init, "Entex", "Pac Man 2 (Entex, red Pacman)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, einvader2, 0,        0, einvader2,einvader2,einvader2_state,empty_init, "Entex", "Super Space Invader 2 (Entex, black version)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, eturtles,  0,        0, eturtles, eturtles, eturtles_state, empty_init, "Entex", "Turtles (Entex)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, estargte,  0,        0, estargte, estargte, estargte_state, empty_init, "Entex", "Stargate (Entex)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, ghalien,   0,        0, ghalien,  ghalien,  ghalien_state,  empty_init, "Gakken", "Heiankyo Alien (Gakken)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gckong,    0,        0, gckong,   gckong,   gckong_state,   empty_init, "Gakken", "Crazy Kong (Gakken)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gscobra,   0,        0, gscobra,  gscobra,  gscobra_state,  empty_init, "Gakken", "Super Cobra (Gakken, green version)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gdigdug,   0,        0, gdigdug,  gdigdug,  gdigdug_state,  empty_init, "Gakken", "Dig Dug (Gakken)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, mwcbaseb,  0,        0, mwcbaseb, mwcbaseb, mwcbaseb_state, empty_init, "Mattel Electronics", "World Championship Baseball", MACHINE_SUPPORTS_SAVE )
CONS( 1982, msthawk,   0,        0, msthawk,  msthawk,  msthawk_state,  empty_init, "Mattel Electronics", "Star Hawk (Mattel)", MACHINE_SUPPORTS_SAVE )

CONS( 1983, pbqbert,   0,        0, pbqbert,  pbqbert,  pbqbert_state,  empty_init, "Parker Brothers", "Q*Bert (Parker Brothers)", MACHINE_SUPPORTS_SAVE )

CONS( 1982, tmtron,    0,        0, tmtron,   tmtron,   tmtron_state,   empty_init, "Tomy", "Tron (Tomy)", MACHINE_SUPPORTS_SAVE )
CONS( 1982, kingman,   0,        0, kingman,  kingman,  kingman_state,  empty_init, "Tomy", "Kingman", MACHINE_SUPPORTS_SAVE )

CONS( 1981, vinvader,  0,        0, vinvader, vinvader, vinvader_state, empty_init, "VTech", "Invaders (VTech)", MACHINE_SUPPORTS_SAVE )
