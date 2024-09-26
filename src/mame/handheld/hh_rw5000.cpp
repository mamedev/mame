// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Rockwell A/B5000 MCU series handhelds (before PPS-4/1)
Mostly calculators on these MCUs, but also Mattel's first couple of handhelds.

ROM source notes when dumped from another title, but confident it's the same:
- rw18r: Rockwell 8R
- rw24k: Rockwell 14RD-II
- misatk: Mattel Space Alert

*******************************************************************************/

#include "emu.h"

#include "cpu/rw5000/a5000.h"
#include "cpu/rw5000/a5500.h"
#include "cpu/rw5000/a5900.h"
#include "cpu/rw5000/b5000.h"
#include "cpu/rw5000/b5500.h"
#include "cpu/rw5000/b6000.h"
#include "cpu/rw5000/b6100.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "autorace.lh"
#include "gravity.lh"
#include "mbaseb.lh"
#include "mfootb.lh"
#include "misatk.lh"
#include "rw10r.lh"
#include "rw24k.lh"
#include "rw30r.lh"

//#include "hh_rw5000_test.lh" // common test-layout - use external artwork


namespace {

class hh_rw5000_state : public driver_device
{
public:
	hh_rw5000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(power_button);
	template<int Sel> DECLARE_INPUT_CHANGED_MEMBER(switch_next) { if (newval) switch_change(Sel, param, true); }
	template<int Sel> DECLARE_INPUT_CHANGED_MEMBER(switch_prev) { if (newval) switch_change(Sel, param, false); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices
	required_device<rw5000_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<9> m_inputs; // max 9

	u16 m_inp_mux = 0;

	// MCU output pin state
	u16 m_str = 0;
	u16 m_seg = 0;

	u8 read_inputs(int columns);
	void switch_change(int sel, u32 mask, bool next);
};


// machine start/reset

void hh_rw5000_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_str));
	save_item(NAME(m_seg));
}

void hh_rw5000_state::machine_reset()
{
}



/*******************************************************************************

  Helper Functions

*******************************************************************************/

// generic input handlers

u8 hh_rw5000_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (BIT(m_inp_mux, i))
			ret |= m_inputs[i]->read();

	return ret;
}

void hh_rw5000_state::switch_change(int sel, u32 mask, bool next)
{
	// config switches (for direct control)
	ioport_field *inp = m_inputs[sel]->field(mask);

	if (next && inp->has_next_setting())
		inp->select_next_setting();
	else if (!next && inp->has_previous_setting())
		inp->select_previous_setting();
}

INPUT_CHANGED_MEMBER(hh_rw5000_state::power_button)
{
	// power button or switch
	bool power = (param) ? (bool(param - 1)) : !newval;

	if (!power && m_display != nullptr)
		m_display->clear();
	m_maincpu->set_input_line(INPUT_LINE_RESET, power ? CLEAR_LINE : ASSERT_LINE);
}



/*******************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

*******************************************************************************/

/*******************************************************************************

  Mattel Auto Race (model 9879)
  * B6000 MCU (label B6000CA, die label B6000-B)
  * 2-digit 7seg display, 21 other leds, 1-bit sound

  This is Mattel's first electronic handheld game, also the first CPU-based
  handheld game overall. Hardware design (even the MCU) and programming
  was done at Rockwell.

  A European version was released as "Ski Slalom", except it's upside-down.

*******************************************************************************/

class autorace_state : public hh_rw5000_state
{
public:
	autorace_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void autorace(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
};

// handlers

void autorace_state::write_str(u16 data)
{
	m_display->write_my(data);
}

void autorace_state::write_seg(u16 data)
{
	m_display->write_mx(data);
}

// inputs

static INPUT_PORTS_START( autorace )
	PORT_START("IN.0") // KB
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) // does not auto-center on real device
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) // "
	PORT_CONFNAME( 0x0c, 0x0c, "Gear" )
	PORT_CONFSETTING(    0x0c, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x00, "3" )
	PORT_CONFSETTING(    0x08, "4" )

	PORT_START("IN.1") // DIN
	PORT_CONFNAME( 0x01, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START("POWER") // power switch
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_rw5000_state, power_button, 0) PORT_NAME("Start / Reset")

	PORT_START("SWITCH") // fake
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_rw5000_state, switch_prev<0>, 0x0c) PORT_NAME("Gear Switch Down")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_rw5000_state, switch_next<0>, 0x0c) PORT_NAME("Gear Switch Up")
INPUT_PORTS_END

// config

void autorace_state::autorace(machine_config &config)
{
	// basic machine hardware
	B6000(config, m_maincpu, 160000); // approximation
	m_maincpu->write_str().set(FUNC(autorace_state::write_str));
	m_maincpu->write_seg().set(FUNC(autorace_state::write_seg));
	m_maincpu->read_kb().set_ioport("IN.0");
	m_maincpu->read_din().set_ioport("IN.1");
	m_maincpu->write_spk().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x180, 0x7f);
	m_display->set_bri_levels(0.02, 0.2); // player led is brighter
	config.set_default_layout(layout_autorace);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( autorace )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "b6000ca", 0x000, 0x200, CRC(a112c928) SHA1(aa5d0b46a08e2460081d4148cf254dc5ec53817e) )
ROM_END





/*******************************************************************************

  Mattel Missile Attack (model 2048) / Space Alert (model 2448)
  * B6000 MCU (label B6001CA/EA, die label B6001)
  * 2-digit 7seg display, 21 other leds, 1-bit sound

  The initial release was called Missile Attack, it didn't sell well (Mattel
  blamed it on NBC for refusing to air their commercial). They changed the
  title/setting and rereleased it as "Space Alert" (aka "Battlestar Galactica:
  Space Alert"). In 1980, they advertised another rerelease, this time as
  "Flash Gordon", but that didn't come out.

*******************************************************************************/

class misatk_state : public hh_rw5000_state
{
public:
	misatk_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void misatk(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
};

// handlers

void misatk_state::write_str(u16 data)
{
	m_display->write_my(data);
}

void misatk_state::write_seg(u16 data)
{
	m_display->write_mx(data);
}

// inputs

static INPUT_PORTS_START( misatk )
	PORT_START("IN.0") // KB
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) // does not auto-center on real device
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1") // DIN
	PORT_CONFNAME( 0x01, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START("POWER") // power switch
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_rw5000_state, power_button, 0) PORT_NAME("Arm / Off")
INPUT_PORTS_END

// config

void misatk_state::misatk(machine_config &config)
{
	// basic machine hardware
	B6000(config, m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(misatk_state::write_str));
	m_maincpu->write_seg().set(FUNC(misatk_state::write_seg));
	m_maincpu->read_kb().set_ioport("IN.0");
	m_maincpu->read_din().set_ioport("IN.1");
	m_maincpu->write_spk().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x180, 0x7f);
	m_display->set_bri_levels(0.015, 0.15); // player led is brighter
	config.set_default_layout(layout_misatk);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( misatk )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "b6001ea", 0x000, 0x200, CRC(56564b79) SHA1(6f33f57ea312cb2018fb59f72eaff3a9642e74a2) )
ROM_END





/*******************************************************************************

  Mattel Football (model 2024)
  * B6100 MCU (label B6100EB/-15, die label B6100 A)
  * 7-digit 7seg display, 27 other leds, 1-bit sound

  When Football II came out, they renamed this one to Football I.

*******************************************************************************/

class mfootb_state : public hh_rw5000_state
{
public:
	mfootb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void mfootb(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(score_button) { update_display(); }

private:
	void update_display();
	void write_str(u16 data);
	void write_seg(u16 data);
};

// handlers

void mfootb_state::update_display()
{
	// 4th digit DP is from the SCORE button
	u8 dp = (m_inputs[1]->read() & 2) ? 0x80 : 0;
	m_display->matrix(m_str, (m_seg << 1 & 0x700) | dp | (m_seg & 0x7f));
}

void mfootb_state::write_str(u16 data)
{
	m_str = data;
	update_display();
}

void mfootb_state::write_seg(u16 data)
{
	m_seg = data;
	update_display();
}

// inputs

static INPUT_PORTS_START( mfootb )
	PORT_START("IN.0") // KB
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Forward")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Kick")

	PORT_START("IN.1") // DIN
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" ) // PRO 1
	PORT_CONFSETTING(    0x00, "2" ) // PRO 2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Score") PORT_CHANGED_MEMBER(DEVICE_SELF, mfootb_state, score_button, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Status")
	PORT_CONFNAME( 0x08, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

// config

void mfootb_state::mfootb(machine_config &config)
{
	// basic machine hardware
	B6100(config, m_maincpu, 280000); // approximation
	m_maincpu->write_str().set(FUNC(mfootb_state::write_str));
	m_maincpu->write_seg().set(FUNC(mfootb_state::write_seg));
	m_maincpu->read_kb().set_ioport("IN.0");
	m_maincpu->read_din().set_ioport("IN.1");
	m_maincpu->write_spk().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 11);
	m_display->set_segmask(0x7f, 0x7f);
	m_display->set_segmask(0x08, 0xff); // only one digit has DP
	m_display->set_bri_levels(0.02, 0.2); // player led is brighter
	config.set_default_layout(layout_mfootb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mfootb )
	ROM_REGION( 0x400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b6100eb", 0x000, 0x300, CRC(5b27620f) SHA1(667ff6cabced89ef4ad848b73d66a06526edc5e6) )
	ROM_CONTINUE(        0x380, 0x080 )
ROM_END





/*******************************************************************************

  Mattel Baseball (model 2942)
  * B6100 MCU (label B6101-12, die label B6101 A)
  * 4-digit 7seg display, 28 other leds, 1-bit sound

*******************************************************************************/

class mbaseb_state : public hh_rw5000_state
{
public:
	mbaseb_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void mbaseb(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
};

// handlers

void mbaseb_state::write_str(u16 data)
{
	m_display->write_my(data);
}

void mbaseb_state::write_seg(u16 data)
{
	m_display->write_mx(bitswap<10>(data,7,8,9,6,5,4,3,2,1,0));
}

// inputs

static INPUT_PORTS_START( mbaseb )
	PORT_START("IN.0") // KB
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pitch")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hit")

	PORT_START("IN.1") // DIN
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "1" ) // PRO 1
	PORT_CONFSETTING(    0x00, "2" ) // PRO 2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Score")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Run")
	PORT_CONFNAME( 0x08, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

// config

void mbaseb_state::mbaseb(machine_config &config)
{
	// basic machine hardware
	B6100(config, m_maincpu, 280000); // approximation
	m_maincpu->write_str().set(FUNC(mbaseb_state::write_str));
	m_maincpu->write_seg().set(FUNC(mbaseb_state::write_seg));
	m_maincpu->read_kb().set_ioport("IN.0");
	m_maincpu->read_din().set_ioport("IN.1");
	m_maincpu->write_spk().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 10);
	m_display->set_segmask(0x170, 0x7f);
	m_display->set_segmask(0x110, 0xff); // 2 digits have DP
	config.set_default_layout(layout_mbaseb);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( mbaseb )
	ROM_REGION( 0x400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b6101-12", 0x000, 0x300, CRC(7720ddcc) SHA1(cd43126db7c6262f659f325aa58420e1a8a1a659) )
	ROM_CONTINUE(         0x380, 0x080 )
ROM_END





/*******************************************************************************

  Mattel Gravity (model 8291)
  * B6100 MCU (label B6102-11, die label B6102 A)
  * 3-digit 7seg display, 27 other leds, 1-bit sound

  It was advertised as "Catastrophe", but went unreleased. It got sold later
  as "Gravity", with a less catastrophic setting.

  The game is basically 3 mini games in 1:
  - Juggling (Rumbling Rocks in Catastrophe)
  - Coin Drop (Quake Shock in Catastrophe)
  - Docking (Meteorite Shower in Catstrophe)

*******************************************************************************/

class gravity_state : public hh_rw5000_state
{
public:
	gravity_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void gravity(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
};

// handlers

void gravity_state::write_str(u16 data)
{
	m_display->write_my(data);
}

void gravity_state::write_seg(u16 data)
{
	m_display->write_mx(data);
}

// inputs

static INPUT_PORTS_START( gravity )
	PORT_START("IN.0") // KB
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // DIN
	PORT_CONFNAME( 0x08, 0x00, "Factory Test" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

// config

void gravity_state::gravity(machine_config &config)
{
	// basic machine hardware
	B6100(config, m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(gravity_state::write_str));
	m_maincpu->write_seg().set(FUNC(gravity_state::write_seg));
	m_maincpu->read_kb().set_ioport("IN.0");
	m_maincpu->read_din().set_ioport("IN.1");
	m_maincpu->write_spk().set(m_speaker, FUNC(speaker_sound_device::level_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 10);
	m_display->set_segmask(0x1c0, 0x7f);
	config.set_default_layout(layout_gravity);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( gravity )
	ROM_REGION( 0x400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b6102-11", 0x000, 0x300, CRC(532f332e) SHA1(593224d3a2a5b6022f0d73b6e6fb9093d2d54f68) )
	ROM_CONTINUE(         0x380, 0x080 )
ROM_END





/*******************************************************************************

  Rockwell 10R
  * A5000 MCU (label A5000PA, die label A5000)
  * 8-digit 7seg LED display

  Rockwell 12R "Square Root"
  * A5000 MCU (label A5001, die label A5001)
  * rest is same as 10R

  12R supports square root by pressing × after ÷ (or the other way around).

*******************************************************************************/

class rw10r_state : public hh_rw5000_state
{
public:
	rw10r_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void rw10r(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
	u8 read_kb();
};

// handlers

void rw10r_state::write_str(u16 data)
{
	// STR0-STR7: digit select
	// STR4-STR7: input mux
	m_display->write_my(data);
	m_inp_mux = data >> 4;
}

void rw10r_state::write_seg(u16 data)
{
	// SEG0-SEG7: digit segment data
	m_display->write_mx(bitswap<8>(data,0,7,6,5,4,3,2,1));
}

u8 rw10r_state::read_kb()
{
	// KB: multiplexed inputs
	return read_inputs(4);
}

// inputs

static INPUT_PORTS_START( rw10r )
	PORT_START("IN.0") // STR4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE/C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("+=")

	PORT_START("IN.1") // STR5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.2") // STR6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")

	PORT_START("IN.3") // STR7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")
INPUT_PORTS_END

// config

void rw10r_state::rw10r(machine_config &config)
{
	// basic machine hardware
	A5000(config, m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(rw10r_state::write_str));
	m_maincpu->write_seg().set(FUNC(rw10r_state::write_seg));
	m_maincpu->read_kb().set(FUNC(rw10r_state::read_kb));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_rw10r);
}

// roms

ROM_START( rw10r )
	ROM_REGION( 0x200, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "a5000pa", 0x000, 0x0c0, CRC(db6cbbea) SHA1(3757f303a84e412e8763c4ec2170c9badc172454) )
	ROM_CONTINUE(        0x100, 0x100 )
ROM_END

ROM_START( rw12r )
	ROM_REGION( 0x200, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "a5001", 0x000, 0x0c0, CRC(4337a46b) SHA1(70f4683eac0afa31a6cd9dd44fdf1b6680575f67) )
	ROM_CONTINUE(      0x100, 0x100 )
ROM_END





/*******************************************************************************

  Rockwell 8R "Automatic Percent", Rockwell 18R "Memory"
  * B5000 MCU (label B5000CC, die label B5000)
  * 8-digit 7seg LED display

  This MCU was used in Rockwell 8R, 18R, and 9TR. It was also sold by
  Tandy (Radio Shack) as EC-220.

  8R/9TR doesn't have the memory store/recall buttons.

*******************************************************************************/

class rw18r_state : public hh_rw5000_state
{
public:
	rw18r_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void rw18r(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
	u8 read_kb();
};

// handlers

void rw18r_state::write_str(u16 data)
{
	// STR0-STR7: digit select
	// STR4-STR8: input mux
	m_display->write_my(data);
	m_inp_mux = data >> 4;
}

void rw18r_state::write_seg(u16 data)
{
	// SEG0-SEG7: digit segment data
	m_display->write_mx(bitswap<8>(data,0,7,6,5,4,3,2,1));
}

u8 rw18r_state::read_kb()
{
	// KB: multiplexed inputs
	return read_inputs(5);
}

// inputs

static INPUT_PORTS_START( rw18r )
	PORT_START("IN.0") // STR4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE/C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.1") // STR5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.2") // STR6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")

	PORT_START("IN.3") // STR7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")

	PORT_START("IN.4") // STR8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("STO") // unpopulated on 8R
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("RCL") // "
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
INPUT_PORTS_END

// config

void rw18r_state::rw18r(machine_config &config)
{
	// basic machine hardware
	B5000(config, m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(rw18r_state::write_str));
	m_maincpu->write_seg().set(FUNC(rw18r_state::write_seg));
	m_maincpu->read_kb().set(FUNC(rw18r_state::read_kb));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_rw10r);
}

// roms

ROM_START( rw18r )
	ROM_REGION( 0x200, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b5000cc", 0x000, 0x0c0, CRC(ace32614) SHA1(23cf11acf2e73ce2dfc165cb87f86fab15f69ff7) )
	ROM_CONTINUE(        0x100, 0x100 )
ROM_END





/*******************************************************************************

  Rockwell 30R "Slide Rule Memory"
  * B5500 MCU (label B5500PA, die label B5500)
  * 9-digit 7seg LED display

  Rockwell 31R "Slide Rule Memory"
  * A5500 MCU (label A5502PA, die label A5500)
  * rest is same as 30R

  30R and 31R have the exact same functionality, even though they are on
  different MCUs. There's also a 30R version on an A4600 MCU.

*******************************************************************************/

class rw30r_state : public hh_rw5000_state
{
public:
	rw30r_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void rw30r(machine_config &config);
	void rw31r(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
	u8 read_kb();
};

// handlers

void rw30r_state::write_str(u16 data)
{
	// STR0-STR8: digit select
	// STR4-STR8: input mux
	m_display->write_my(data);
	m_inp_mux = data >> 4;
}

void rw30r_state::write_seg(u16 data)
{
	// SEG0-SEG7: digit segment data
	m_display->write_mx(bitswap<8>(data,0,7,6,5,4,3,2,1));
}

u8 rw30r_state::read_kb()
{
	// KB: multiplexed inputs
	return read_inputs(5);
}

// inputs

static INPUT_PORTS_START( rw30r )
	PORT_START("IN.0") // STR4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(". / +/-")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("F / CF") // function
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+ / M+")

	PORT_START("IN.1") // STR5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("- / M-")

	PORT_START("IN.2") // STR6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"× / x²")

	PORT_START("IN.3") // STR7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷ / 1/x")

	PORT_START("IN.4") // STR8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("C / MC")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME(u8"X\u2194Y / X\u2194M") // U+2194 = ↔
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME(u8"% / u221ax") // U+221A = √
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("= / MR")
INPUT_PORTS_END

// config

void rw30r_state::rw30r(machine_config &config)
{
	// basic machine hardware
	B5500(config, m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(rw30r_state::write_str));
	m_maincpu->write_seg().set(FUNC(rw30r_state::write_seg));
	m_maincpu->read_kb().set(FUNC(rw30r_state::read_kb));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1ff, 0xff);
	config.set_default_layout(layout_rw30r);
}

void rw30r_state::rw31r(machine_config &config)
{
	rw30r(config);

	// basic machine hardware
	A5500(config.replace(), m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(rw30r_state::write_str));
	m_maincpu->write_seg().set(FUNC(rw30r_state::write_seg));
	m_maincpu->read_kb().set(FUNC(rw30r_state::read_kb));
}

// roms

ROM_START( rw30r )
	ROM_REGION( 0x400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b5500pa", 0x000, 0x280, CRC(937dd695) SHA1(bc286209943f49e9acccbf319b87c6ec24386894) )
	ROM_CONTINUE(        0x380, 0x080 )
ROM_END

ROM_START( rw31r )
	ROM_REGION( 0x400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "a5502pa", 0x000, 0x280, CRC(6ee0b30a) SHA1(42e5d1e29bdef4b4faaafee3592cacc0e66b982f) )
	ROM_CONTINUE(        0x380, 0x080 )
ROM_END





/*******************************************************************************

  Rockwell 24K aka "the 24K" (see below for more)
  * A5900 MCU (label A5901CA/A5903CB, die label A59__)
  * 9-digit 7seg VFD

  This MCU was used in Rockwell 14RD-II, 24RD-II, 24K, 24K II, 24MS, and
  probably also in 14RD, 24RD, 22K.

  14RD has 6 less buttons than 24RD, the non-II versions have LED(s) instead
  of a 9th digit for minus sign and memory status. 24K has an extra button
  for register exchange. The difference between 24K and 24K II is unknown.

*******************************************************************************/

class rw24k_state : public hh_rw5000_state
{
public:
	rw24k_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_rw5000_state(mconfig, type, tag)
	{ }

	void rw24k(machine_config &config);

private:
	void write_str(u16 data);
	void write_seg(u16 data);
	u8 read_kb();
};

// handlers

void rw24k_state::write_str(u16 data)
{
	// STR0-STR8: digit select, input mux
	m_display->write_my(data);
	m_inp_mux = data;
}

void rw24k_state::write_seg(u16 data)
{
	// SEG0-SEG7: digit segment data
	m_display->write_mx(bitswap<8>(data,0,7,6,5,4,3,2,1));
}

u8 rw24k_state::read_kb()
{
	// KB: multiplexed inputs
	return read_inputs(9);
}

// inputs

static INPUT_PORTS_START( rw24k )
	PORT_START("IN.0") // STR0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("+/-") // unpopulated on 14RD
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("M+") // "

	PORT_START("IN.1") // STR1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("M-") // "
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // STR2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // STR3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("MC") // "
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME(u8"\u221a") // " - U+221A = √
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("MR") // "

	PORT_START("IN.4") // STR4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.5") // STR5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.6") // STR6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")

	PORT_START("IN.7") // STR7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")

	PORT_START("IN.8") // STR8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE/C")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME(u8"\u2194") // register exchange - unpopulated on 14RD/24RD - U+2194 = ↔
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("%")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")
INPUT_PORTS_END

// config

void rw24k_state::rw24k(machine_config &config)
{
	// basic machine hardware
	A5900(config, m_maincpu, 250000); // approximation
	m_maincpu->write_str().set(FUNC(rw24k_state::write_str));
	m_maincpu->write_seg().set(FUNC(rw24k_state::write_seg));
	m_maincpu->read_kb().set(FUNC(rw24k_state::read_kb));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	m_display->set_segmask(0x1ff, 0xff);
	config.set_default_layout(layout_rw24k);
}

// roms

ROM_START( rw24k )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "a5901ca", 0x000, 0x200, CRC(00de7764) SHA1(0f24add4b6d2660aad63ddd4d0003d59a0e39df6) )
ROM_END



} // anonymous namespace

/*******************************************************************************

  Game driver(s)

*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1976, autorace,  0,       0,      autorace,  autorace,  autorace_state,  empty_init, "Mattel Electronics", "Auto Race", MACHINE_SUPPORTS_SAVE )
SYST( 1977, misatk,    0,       0,      misatk,    misatk,    misatk_state,    empty_init, "Mattel Electronics", "Missile Attack / Space Alert", MACHINE_SUPPORTS_SAVE )
SYST( 1977, mfootb,    0,       0,      mfootb,    mfootb,    mfootb_state,    empty_init, "Mattel Electronics", "Football (Mattel)", MACHINE_SUPPORTS_SAVE )
SYST( 1978, mbaseb,    0,       0,      mbaseb,    mbaseb,    mbaseb_state,    empty_init, "Mattel Electronics", "Baseball (Mattel)", MACHINE_SUPPORTS_SAVE )
SYST( 1980, gravity,   0,       0,      gravity,   gravity,   gravity_state,   empty_init, "Mattel Electronics", "Gravity (Mattel)", MACHINE_SUPPORTS_SAVE )

SYST( 1974, rw10r,     0,       0,      rw10r,     rw10r,     rw10r_state,     empty_init, "Rockwell", "10R (Rockwell)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1975, rw12r,     0,       0,      rw10r,     rw10r,     rw10r_state,     empty_init, "Rockwell", "12R: Square Root", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1975, rw18r,     0,       0,      rw18r,     rw18r,     rw18r_state,     empty_init, "Rockwell", "18R: Memory", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1974, rw30r,     0,       0,      rw30r,     rw30r,     rw30r_state,     empty_init, "Rockwell", "30R: Slide Rule Memory (B5500 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1975, rw31r,     rw30r,   0,      rw31r,     rw30r,     rw30r_state,     empty_init, "Rockwell", "31R: Slide Rule Memory", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
SYST( 1976, rw24k,     0,       0,      rw24k,     rw24k,     rw24k_state,     empty_init, "Rockwell", "24K (Rockwell)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
