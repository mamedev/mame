// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton
/***************************************************************************

Mitsubishi MELPS 4 MCU tabletops/handhelds or other simple devices,
most of them are VFD electronic games/toys.

TODO:
- dump/add Gakken version of Frogger
- get rid of hardcoded color overlay from SVGs, use MAME internal artwork

***************************************************************************/

#include "emu.h"

#include "cpu/melps4/m58846.h"
#include "video/pwm.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

//#include "hh_melps4_test.lh" // common test-layout - no svg artwork(yet), use external artwork


class hh_melps4_state : public driver_device
{
public:
	hh_melps4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<m58846_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<4> m_inputs; // max 4

	// misc common
	u16 m_inp_mux = 0;              // multiplexed inputs mask

	u32 m_grid = 0;                 // VFD current row data
	u32 m_plate = 0;                // VFD current column data

	u8 read_inputs(int columns);
};


// machine start/reset

void hh_melps4_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_grid));
	save_item(NAME(m_plate));
}

void hh_melps4_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_melps4_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}

INPUT_CHANGED_MEMBER(hh_melps4_state::reset_button)
{
	// for when reset button is directly tied to MCU reset pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Coleco Frogger (manufactured in Japan, licensed from Sega)
  * PCB label Coleco Frogger Code No. 01-81543, KS-003282 Japan
  * Mitsubishi M58846-701P MCU
  * cyan/red/green VFD display Itron CP5090GLR R1B, with partial color overlay

  Gakken / Konami Frogger
  * PCB label Konami Gakken KH-8201D
  * Mitsubishi M58846-700P MCU (Konami logo on it)
  * cyan/red/green VFD display, with partial color overlay

***************************************************************************/

class cfrogger_state : public hh_melps4_state
{
public:
	cfrogger_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_melps4_state(mconfig, type, tag)
	{ }

	void cfrogger(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	u16 input_r();
};

// handlers

void cfrogger_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,0,1,2,3,4,5,6,7,8,9,10,11);
	u16 plate = bitswap<16>(m_plate,12,4,13,5,14,6,15,7,3,11,2,10,1,9,0,8);
	m_display->matrix(grid, plate);
}

void cfrogger_state::plate_w(offs_t offset, u8 data)
{
	// F0,F1: input mux
	if (offset == MELPS4_PORTF)
		m_inp_mux = data & 3;

	// Sx,Fx,Gx: vfd plate
	int mask = (offset == MELPS4_PORTS) ? 0xff : 0xf; // port S is 8-bit
	int shift = (offset == MELPS4_PORTS) ? 0 : (offset + 1) * 4;
	m_plate = (m_plate & ~(mask << shift)) | (data << shift);
	update_display();
}

void cfrogger_state::grid_w(u16 data)
{
	// D0-D11: vfd grid
	m_grid = data;
	update_display();
}

WRITE_LINE_MEMBER(cfrogger_state::speaker_w)
{
	// T: speaker out
	m_speaker->level_w(state);
}

u16 cfrogger_state::input_r()
{
	// K0,K1: multiplexed inputs
	// K2: N/C
	// K3: fixed input
	return (m_inputs[2]->read() & 8) | (read_inputs(2) & 3);
}

// config

static INPUT_PORTS_START( cfrogger )
	PORT_START("IN.0") // F0 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // F1 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN.2") // K3
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_melps4_state, reset_button, 0)
INPUT_PORTS_END

void cfrogger_state::cfrogger(machine_config &config)
{
	// basic machine hardware
	M58846(config, m_maincpu, 600_kHz_XTAL);
	m_maincpu->read_k().set(FUNC(cfrogger_state::input_r));
	m_maincpu->write_s().set(FUNC(cfrogger_state::plate_w));
	m_maincpu->write_f().set(FUNC(cfrogger_state::plate_w));
	m_maincpu->write_g().set(FUNC(cfrogger_state::plate_w));
	m_maincpu->write_d().set(FUNC(cfrogger_state::grid_w));
	m_maincpu->write_t().set(FUNC(cfrogger_state::speaker_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(500, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(12, 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( cfrogger )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m58846-701p", 0x0000, 0x1000, CRC(ba52a242) SHA1(7fa53b617f4bb54be32eb209e9b88131e11cb518) )

	ROM_REGION( 786255, "screen", 0)
	ROM_LOAD( "cfrogger.svg", 0, 786255, CRC(d8d6e2b6) SHA1(bc9a0260b211ed07021dfe1cc19a993569f4c544) )
ROM_END





/***************************************************************************

  Gakken / Konami Jungler (manufactured in Japan)
  * PCB label Konami Gakken GR503
  * Mitsubishi M58846-702P MCU
  * cyan/red/green VFD display Itron CP5143GLR SGA, with light-yellow color overlay

***************************************************************************/

class gjungler_state : public hh_melps4_state
{
public:
	gjungler_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_melps4_state(mconfig, type, tag)
	{ }

	void gjungler(machine_config &config);

private:
	void update_display();
	void plate_w(offs_t offset, u8 data);
	void grid_w(u16 data);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	u16 input_r();
};

// handlers

void gjungler_state::update_display()
{
	u16 grid = bitswap<16>(m_grid,15,14,13,12,11,10,9,8,7,6,5,4,3,2,0,1);
	u32 plate = bitswap<24>(m_plate,23,22,21,20,19,18,8,9,10,11,13,16,15,14,13,12,7,0,6,1,5,2,4,3) | 0x2000;
	m_display->matrix(grid, plate);
}

void gjungler_state::plate_w(offs_t offset, u8 data)
{
	// G0,G1: input mux
	if (offset == MELPS4_PORTG)
		m_inp_mux = data & 3;

	// Sx,Fx,Gx,U: vfd plate
	int mask = (offset == MELPS4_PORTS) ? 0xff : 0xf; // port S is 8-bit
	int shift = (offset == MELPS4_PORTS) ? 0 : (offset + 1) * 4;
	m_plate = (m_plate & ~(mask << shift)) | (data << shift);
	update_display();
}

void gjungler_state::grid_w(u16 data)
{
	// D0-D11: vfd grid
	m_grid = data;
	update_display();
}

WRITE_LINE_MEMBER(gjungler_state::speaker_w)
{
	// T: speaker out
	m_speaker->level_w(state);
}

u16 gjungler_state::input_r()
{
	// K0,K1: multiplexed inputs
	// K2,K3: fixed inputs
	return (m_inputs[2]->read() & 0xc) | (read_inputs(2) & 3);
}

// config

static INPUT_PORTS_START( gjungler )
	PORT_START("IN.0") // G0 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN.1") // G1 port K0,K1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN.2") // K2,K3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_CONFNAME( 0x08, 0x00, "Game Mode" )
	PORT_CONFSETTING(    0x00, "A" )
	PORT_CONFSETTING(    0x08, "B" )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_melps4_state, reset_button, 0)
INPUT_PORTS_END

void gjungler_state::gjungler(machine_config &config)
{
	// basic machine hardware
	M58846(config, m_maincpu, 600_kHz_XTAL);
	m_maincpu->read_k().set(FUNC(gjungler_state::input_r));
	m_maincpu->write_s().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_f().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_g().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_u().set(FUNC(gjungler_state::plate_w));
	m_maincpu->write_d().set(FUNC(gjungler_state::grid_w));
	m_maincpu->write_t().set(FUNC(gjungler_state::speaker_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(481, 1080);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_display).set_size(12, 18);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gjungler )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m58846-702p", 0x0000, 0x1000, CRC(94ab7060) SHA1(3389bc115d1df8d01a30611fa9e95a900d32b29b) )

	ROM_REGION( 419696, "screen", 0)
	ROM_LOAD( "gjungler.svg", 0, 419696, CRC(c5f6d1f2) SHA1(5032f35326ca689c8e329f760e380cdc9f5dff86) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1982, cfrogger, 0,      0, cfrogger, cfrogger, cfrogger_state, empty_init, "Coleco / Konami", "Frogger (Coleco)", MACHINE_SUPPORTS_SAVE )

CONS( 1982, gjungler, 0,      0, gjungler, gjungler, gjungler_state, empty_init, "Gakken / Konami", "Jungler (Gakken)", MACHINE_SUPPORTS_SAVE )
