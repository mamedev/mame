// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Matsushita (Panasonic) MN1400 handhelds

TODO:
- WIP
- NO_DUMP: it's decapped, but rom layout is not fully verified yet

*******************************************************************************/

#include "emu.h"

#include "cpu/mn1400/mn1400.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

#include "hh_mn1400_test.lh" // common test-layout - use external artwork


namespace {

class hh_mn1400_state : public driver_device
{
public:
	hh_mn1400_state(const machine_config &mconfig, device_type type, const char *tag) :
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
	required_device<mn1400_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<4> m_inputs; // max 4

	u8 m_inp_mux = 0; // multiplexed inputs mask

	// MCU output pins state
	u16 m_c = 0;      // C pins
	u8 m_d = 0;       // D pins
	u8 m_e = 0;       // E pins

	u8 read_inputs(int columns);
};


// machine start/reset

void hh_mn1400_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_c));
	save_item(NAME(m_d));
	save_item(NAME(m_e));
}

void hh_mn1400_state::machine_reset()
{
	read_inputs(0);
}



/*******************************************************************************

  Helper Functions

*******************************************************************************/

// generic input handlers

u8 hh_mn1400_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inputs[i]->read();

	return ret;
}



/*******************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

*******************************************************************************/

/*******************************************************************************

  Lakeside Computer Perfection
  * MN1400ML MCU (die label: 1400 ML-0)
  * 10 LEDs, 2-bit sound

*******************************************************************************/

class compperf_state : public hh_mn1400_state
{
public:
	compperf_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_mn1400_state(mconfig, type, tag)
	{ }

	void compperf(machine_config &config);

private:
};

// handlers

// inputs

static INPUT_PORTS_START( compperf )
INPUT_PORTS_END

// config

void compperf_state::compperf(machine_config &config)
{
	// basic machine hardware
	MN1400(config, m_maincpu, 300000);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1, 10);
	config.set_default_layout(layout_hh_mn1400_test);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( compperf )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mn1400ml", 0x0000, 0x0400, NO_DUMP )
ROM_END



} // anonymous namespace

/*******************************************************************************

  Game driver(s)

*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, compperf, 0,      0,      compperf, compperf, compperf_state, empty_init, "Lakeside", "Computer Perfection", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
