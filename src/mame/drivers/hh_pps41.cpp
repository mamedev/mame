// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, Kevin Horton
/***************************************************************************

  Rockwell PPS-4/1 MCU series handhelds

  TODO:
  - WIP

***************************************************************************/

#include "emu.h"

#include "cpu/pps41/mm75.h"
#include "cpu/pps41/mm76.h"
#include "video/pwm.h"
#include "sound/spkrdev.h"
#include "speaker.h"

// internal artwork
#include "mastmind.lh"

//#include "hh_pps41_test.lh" // common test-layout - use external artwork


class hh_pps41_state : public driver_device
{
public:
	hh_pps41_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// devices
	required_device<pps41_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<8> m_inputs; // max 8

	u16 m_inp_mux = 0;

	// MCU output pin state
	//..

	u8 read_inputs(int columns);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_pps41_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
}

void hh_pps41_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_pps41_state::read_inputs(int columns)
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

  Invicta Electronic Master Mind
  * x

  Invicta is the owner of the Mastermind game rights. The back of the unit
  says (C) 1977, but this electronic handheld version came out in 1979.

***************************************************************************/

class mastmind_state : public hh_pps41_state
{
public:
	mastmind_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_pps41_state(mconfig, type, tag)
	{ }

	void mastmind(machine_config &config);
};

// handlers

// config

static INPUT_PORTS_START( mastmind )
INPUT_PORTS_END

void mastmind_state::mastmind(machine_config &config)
{
	/* basic machine hardware */
	MM75(config, m_maincpu, 100000); // approximation

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(3, 0xff);
	config.set_default_layout(layout_mastmind);

	/* no sound! */
}

// roms

ROM_START( mastmind )
	ROM_REGION( 0x0400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mm75_a7525-11", 0x0000, 0x0200, CRC(39dbdd50) SHA1(72fa5781e9df62d91d57437ded2931fab8253c3c) )
	ROM_CONTINUE(              0x0380, 0x0080 )

	ROM_REGION( 314, "maincpu:opla", 0 )
	ROM_LOAD( "mm76_mastmind_output.pla", 0, 314, CRC(84a3a6f2) SHA1(a3baf9a174a02d186769a1a2d81982e6dbdcf1ed) )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT  CMP MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, mastmind, 0,       0, mastmind, mastmind, mastmind_state, empty_init, "Invicta Plastics", "Electronic Master Mind (Invicta)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
