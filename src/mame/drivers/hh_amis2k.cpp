// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/***************************************************************************

AMI S2000 series handhelds or other simple devices.

TODO:
- were any other handhelds with this MCU released?
- wildfire sound can be improved, volume decay should be more steep at the start,
  and the pitch sounds wrong too (latter is an MCU emulation problem)

***************************************************************************/

#include "emu.h"
#include "cpu/amis2000/amis2000.h"
#include "video/pwm.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "speaker.h"

// internal artwork
#include "wildfire.lh"

//#include "hh_amis2k_test.lh" // common test-layout - use external artwork


class hh_amis2k_state : public driver_device
{
public:
	hh_amis2k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// devices
	required_device<amis2000_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<4> m_inputs; // max 4

	// misc common
	u16 m_a = 0;                    // MCU address bus
	u8 m_d = 0;                     // MCU data bus
	int m_f = 0;                    // MCU F_out pin
	u16 m_inp_mux = 0;              // multiplexed inputs mask

	u8 read_inputs(int columns);
};


// machine start/reset

void hh_amis2k_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_a));
	save_item(NAME(m_d));
	save_item(NAME(m_f));
	save_item(NAME(m_inp_mux));
}

void hh_amis2k_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_amis2k_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

***************************************************************************/

namespace {

/***************************************************************************

  Parker Brothers Wildfire, by Bob and Holly Doyle (prototype), and Garry Kitchen
  * AMI S2150, labeled C10641
  * RC circuit for speaker volume decay (see patent US4334679 FIG.5,
    the 2 resistors at A12 are 10K and the cap is 4.7uF)

  This is an electronic handheld pinball game. It has dozens of small leds
  to create the illusion of a moving ball, and even the flippers are leds.
  A drawing of a pinball table is added as overlay.

  led translation table: led Lzz from patent US4334679 FIG.4* = MAME y.x:
  *note: 2 mistakes in it: L19 between L12 and L14 should be L13, and L84 should of course be L48

    0 = -      10 = 6.6    20 = 4.5    30 = 5.3    40 = 5.7    50 = 11.6
    1 = 10.7   11 = 5.6    21 = 4.4    31 = 4.3    41 = 6.0    51 = 11.5
    2 = 10.0   12 = 6.5    22 = 5.4    32 = 5.2    42 = 7.0    52 = 11.4
    3 = 10.1   13 = 7.5    23 = 6.3    33 = 5.1    43 = 8.0    53 = 11.3
    4 = 10.2   14 = 8.5    24 = 7.3    34 = 11.7   44 = 9.0    60 = 3.6
    5 = 10.3   15 = 9.4    25 = 11.1   35 = 7.1    45 = 6.7    61 = 3.6(!)
    6 = 10.4   16 = 8.4    26 = 9.3    36 = 9.1    46 = 7.7    62 = 3.5
    7 = 10.5   17 = 7.4    27 = 9.2    37 = 5.0    47 = 8.7    63 = 3.5(!)
    8 = 8.6    18 = 11.2   28 = 8.2    38 = 6.1    48 = 9.7    70 = 3.3
    9 = 7.6    19 = 5.5    29 = 11.0   39 = 8.1    49 = -

  NOTE!: MAME external artwork is required

***************************************************************************/

class wildfire_state : public hh_amis2k_state
{
public:
	wildfire_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_amis2k_state(mconfig, type, tag)
	{ }

	void wildfire(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void update_display();
	void write_d(u8 data);
	void write_a(u16 data);
	DECLARE_WRITE_LINE_MEMBER(write_f);

	void speaker_update();
	TIMER_DEVICE_CALLBACK_MEMBER(speaker_decay_sim);
	double m_speaker_volume = 0.0;

	std::vector<double> m_speaker_levels;
};

void wildfire_state::machine_start()
{
	hh_amis2k_state::machine_start();
	save_item(NAME(m_speaker_volume));
}

// handlers

void wildfire_state::speaker_update()
{
	if (~m_a & 0x1000)
		m_speaker_volume = 1.0;

	m_speaker->level_w(m_f * 0x7fff * m_speaker_volume);
}

TIMER_DEVICE_CALLBACK_MEMBER(wildfire_state::speaker_decay_sim)
{
	// volume decays when speaker is off (divisor and timer period determine duration)
	speaker_update();
	m_speaker_volume /= 1.0025;
}

void wildfire_state::update_display()
{
	m_display->matrix(~m_a, m_d);
}

void wildfire_state::write_d(u8 data)
{
	// D0-D7: led/7seg data
	m_d = bitswap<8>(data,7,0,1,2,3,4,5,6);
	update_display();
}

void wildfire_state::write_a(u16 data)
{
	// A0-A2: digit select
	// A3-A11: led select
	m_a = data;
	update_display();

	// A12: speaker on
	speaker_update();
}

WRITE_LINE_MEMBER(wildfire_state::write_f)
{
	// F: speaker out
	m_f = state;
	speaker_update();
}

// config

static INPUT_PORTS_START( wildfire )
	PORT_START("IN.0") // I
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shooter Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// 7seg decoder table differs from default, this one is made by hand
static const u8 wildfire_7seg_table[0x10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, // 0, 1, 2, 3, 4, 5, 6, 7
	0x7f, 0x6f, 0x77, 0x73, 0x39, 0x38, 0x79, 0x40  // 8, 9, ?, P, ?, L, ?, -
};

void wildfire_state::wildfire(machine_config &config)
{
	// basic machine hardware
	AMI_S2152(config, m_maincpu, 850000); // approximation - RC osc. R=?, C=?
	m_maincpu->set_7seg_table(wildfire_7seg_table);
	m_maincpu->read_i().set_ioport("IN.0");
	m_maincpu->write_d().set(FUNC(wildfire_state::write_d));
	m_maincpu->write_a().set(FUNC(wildfire_state::write_a));
	m_maincpu->write_f().set(FUNC(wildfire_state::write_f));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(12, 8);
	m_display->set_segmask(7, 0x7f);
	m_display->set_bri_levels(0.01, 0.1); // bumpers are dimmed
	config.set_default_layout(layout_wildfire);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	TIMER(config, "speaker_decay").configure_periodic(FUNC(wildfire_state::speaker_decay_sim), attotime::from_usec(100));

	// set volume levels (set_output_gain is too slow for sub-frame intervals)
	m_speaker_levels.resize(0x8000);
	for (int i = 0; i < 0x8000; i++)
		m_speaker_levels[i] = double(i) / 32768.0;
	m_speaker->set_levels(0x8000, &m_speaker_levels[0]);
}

// roms

ROM_START( wildfire )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	// Typed in from patent US4334679, data should be correct(it included checksums). 1st half was also dumped/verified with release version.
	ROM_LOAD( "us4341385", 0x0000, 0x0400, CRC(84ac0f1f) SHA1(1e00ddd402acfc2cc267c34eed4b89d863e2144f) )
	ROM_CONTINUE(          0x0600, 0x0200 )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, wildfire, 0,      0, wildfire, wildfire, wildfire_state, empty_init, "Parker Brothers", "Wildfire (patent)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK ) // note: pretty sure that it matches the commercial release
