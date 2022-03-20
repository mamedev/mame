// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/***************************************************************************

Rockwell B5000 MCU series handhelds (before PPS-4/1)
Mostly calculators on these MCUs, but also Mattel's first couple of handhelds.

ROM source notes when dumped from another model, but confident it's the same:
- rw18r: Rockwell 8R

TODO:
- figure out why rw18r doesn't work (ROM dump is good)

***************************************************************************/

#include "emu.h"

#include "cpu/b5000/b5000.h"
#include "cpu/b5000/b6000.h"
#include "cpu/b5000/b6100.h"
#include "video/pwm.h"
#include "sound/spkrdev.h"

#include "speaker.h"

// internal artwork
#include "rw18r.lh"

//#include "hh_b5000_test.lh" // common test-layout - use external artwork


class hh_b5000_state : public driver_device
{
public:
	hh_b5000_state(const machine_config &mconfig, device_type type, const char *tag) :
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
	required_device<b5000_base_device> m_maincpu;
	optional_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<5> m_inputs; // max 5

	u16 m_inp_mux = 0;

	// MCU output pin state
	u16 m_str = 0;
	u16 m_seg = 0;

	u8 read_inputs(int columns);
};


// machine start/reset

void hh_b5000_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_str));
	save_item(NAME(m_seg));
}

void hh_b5000_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// generic input handlers

u8 hh_b5000_state::read_inputs(int columns)
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

  Rockwell 8R, Rockwell 18R
  * B5000 MCU (label B5000CC)
  * 8-digit 7seg display

  This MCU was used in Rockwell 8R, 18R, and 9TR. It was also sold by
  Tandy (Radio Shack) as EC-220.

***************************************************************************/

class rw18r_state : public hh_b5000_state
{
public:
	rw18r_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_b5000_state(mconfig, type, tag)
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

// config

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

void rw18r_state::rw18r(machine_config &config)
{
	// basic machine hardware
	B5000(config, m_maincpu, 240000); // approximation
	m_maincpu->write_str().set(FUNC(rw18r_state::write_str));
	m_maincpu->write_seg().set(FUNC(rw18r_state::write_seg));
	m_maincpu->read_kb().set(FUNC(rw18r_state::read_kb));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 8);
	m_display->set_segmask(0xff, 0xff);
	config.set_default_layout(layout_rw18r);
}

// roms

ROM_START( rw18r )
	ROM_REGION( 0x200, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "b5000cc", 0x000, 0x080, CRC(ace32614) SHA1(23cf11acf2e73ce2dfc165cb87f86fab15f69ff7) )
	ROM_CONTINUE(        0x0c0, 0x140 )
ROM_END



} // anonymous namespace

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME       PARENT  CMP MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
CONS( 1975, rw18r,     0,       0, rw18r,     rw18r,     rw18r_state,     empty_init, "Rockwell", "18R (Rockwell)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
