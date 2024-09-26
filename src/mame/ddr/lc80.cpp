// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/***************************************************************************

LC-80 by VEB Mikroelektronik "Karl Marx" Erfurt

When first started, the screen is blank. Wait about 8 seconds for it to
introduce itself, then you may use it or paste to it. The decimal points
indicate which side of the display you will be updating.

Pasting:
    0-F : as is
    +,- : as is
    ADR : ,
    DAT : .
    EX : X

Test Paste:
    ,2000.11+22+33+44+55+66+77+88+99+,2000
    Now press + to confirm the data has been entered.


Hardware notes (2-ROM version from schematics):
- UD880D CPU (Z80 clone) @ D201, ~900kHz no XTAL
- 2KB ROM (2*U505D @ D202/D203)
- 1KB RAM (2*U214D @ D204/D205)
- 2*UD855D (Z80 PIO clone) @ D206/D207
- UD857D (Z80 CTC clone) @ D208
- cassette port
- 6*7seg display, 2 leds, piezo

The PCB is literally inside a leather map that can be closed like a book.
It has a nice calculator style keypad attached to it.

The memory can be expanded. There's an export version with more RAM/ROM by
default and a faster U880D CPU (3300.0kHz XTAL). It included a chess program
called SC-80 that can be started by executing ADR C800. Press ADR again for
NEW GAME, and use the 1-8 number keys for A1-H8. A keypad overlay was included.
The SC-80 chess engine appears to be the same one as in Chess-Master.


TODO:
- KSD11 switch
- CTC clock inputs ("user bus")
- Most characters are lost when pasting, probably global paste speed problem,
  works fine if you overclock the cpu.
- Add internal artwork for the clickable keypad, and an overlay for SC-80?

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "lc80.lh"


namespace {

class lc80_state : public driver_device
{
public:
	lc80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_pio(*this, "pio%u", 0),
		m_ctc(*this, "ctc"),
		m_display(*this, "display"),
		m_cassette(*this, "cassette"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0U),
		m_halt_led(*this, "halt")
	{ }

	void lc80(machine_config &config);
	void lc80a(machine_config &config);
	void lc80e(machine_config &config);
	void lc80_2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device_array<z80pio_device, 2> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<pwm_display_device> m_display;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<6> m_inputs;
	output_finder<> m_halt_led;

	u8 m_matrix = 0;

	void lc80_mem(address_map &map) ATTR_COLD;
	void lc80a_mem(address_map &map) ATTR_COLD;
	void lc80e_mem(address_map &map) ATTR_COLD;
	void lc80_2_mem(address_map &map) ATTR_COLD;
	void lc80_io(address_map &map) ATTR_COLD;

	void halt_w(int state) { m_halt_led = state; }
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);
	void pio1_pa_w(u8 data);
	u8 pio1_pb_r();
	void pio1_pb_w(u8 data);
	u8 pio2_pb_r();
};


// Memory Maps

void lc80_state::lc80_mem(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x03ff).mirror(0x0400).rom();
	map(0x0800, 0x0bff).mirror(0x0400).rom();
}

void lc80_state::lc80a_mem(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).rom();
}

void lc80_state::lc80_2_mem(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).rom();
}

void lc80_state::lc80e_mem(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x4fff).rom();
}

void lc80_state::lc80_io(address_map &map)
{
	map.global_mask(0x1f);
	map(0x14, 0x17).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x18, 0x1b).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


// Input Ports

INPUT_CHANGED_MEMBER(lc80_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(lc80_state::trigger_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( lc80 )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LD") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ST") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("EX") PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('X')

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DAT") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ADR") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, lc80_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, lc80_state, trigger_nmi, 0)
INPUT_PORTS_END


// Z80-CTC Interface

void lc80_state::ctc_z0_w(int state)
{
}

void lc80_state::ctc_z1_w(int state)
{
}

void lc80_state::ctc_z2_w(int state)
{
}


// Z80-PIO Interface

void lc80_state::pio1_pa_w(u8 data)
{
	/*

	    bit     description

	    PA0     VQE23 segment B
	    PA1     VQE23 segment F
	    PA2     VQE23 segment A
	    PA3     VQE23 segment G
	    PA4     VQE23 segment DP
	    PA5     VQE23 segment C
	    PA6     VQE23 segment E
	    PA7     VQE23 segment D

	*/

	m_display->write_mx(bitswap<8>(~data, 4, 3, 1, 6, 7, 5, 0, 2));
}

u8 lc80_state::pio1_pb_r()
{
	// PB0: tape input
	return (m_cassette->input() < +0.0);
}

void lc80_state::pio1_pb_w(u8 data)
{
	/*

	    bit     description

	    PB0     tape input (pio1_pb_r)
	    PB1     tape output, speaker output, OUT led
	    PB2     digit 0
	    PB3     digit 1
	    PB4     digit 2
	    PB5     digit 3
	    PB6     digit 4
	    PB7     digit 5

	*/

	// tape output
	m_cassette->output(BIT(data, 1) ? +1.0 : -1.0);

	// speaker
	m_speaker->level_w(BIT(~data, 1));

	// 7segs/led/keyboard
	m_display->write_my(~data >> 1);
	m_matrix = ~data >> 2 & 0x3f;
}

u8 lc80_state::pio2_pb_r()
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3
	    PB4     key row 0
	    PB5     key row 1
	    PB6     key row 2
	    PB7     key row 3

	*/

	u8 data = 0;

	for (int i = 0; i < 6; i++)
		if (BIT(m_matrix, i))
			data |= m_inputs[i]->read() << 4;

	return data ^ 0xf0;
}


// Z80 Daisy Chain

static const z80_daisy_config lc80_daisy_chain[] =
{
	{ "ctc" },
	{ "pio1" },
	{ "pio0" },
	{ nullptr }
};


// Machine Initialization

void lc80_state::machine_start()
{
	m_halt_led.resolve();

	// install RAM
	address_space &program = m_maincpu->space(AS_PROGRAM);
	const offs_t mirror = (program.map()->m_globalmask & 0xc000) | 0x1000;
	const offs_t start = 0x2000;
	program.install_ram(start, start + m_ram->size() - 1, mirror, m_ram->pointer());

	// register for state saving
	save_item(NAME(m_matrix));
}


// Machine Driver

void lc80_state::lc80(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 900000); // UD880D
	m_maincpu->set_addrmap(AS_PROGRAM, &lc80_state::lc80_mem);
	m_maincpu->set_addrmap(AS_IO, &lc80_state::lc80_io);
	m_maincpu->halt_cb().set(FUNC(lc80_state::halt_w));
	m_maincpu->set_daisy_config(lc80_daisy_chain);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(1+6, 8);
	m_display->set_segmask(0x3f << 1, 0xff);
	config.set_default_layout(layout_lc80);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	Z80CTC(config, m_ctc, 900000);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(lc80_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(lc80_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(FUNC(lc80_state::ctc_z2_w));

	Z80PIO(config, m_pio[0], 900000);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[0]->out_pa_callback().set(FUNC(lc80_state::pio1_pa_w));
	m_pio[0]->in_pb_callback().set(FUNC(lc80_state::pio1_pb_r));
	m_pio[0]->out_pb_callback().set(FUNC(lc80_state::pio1_pb_w));

	Z80PIO(config, m_pio[1], 900000);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[1]->in_pb_callback().set(FUNC(lc80_state::pio2_pb_r));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	RAM(config, m_ram).set_extra_options("1K,2K,3K,4K");
	m_ram->set_default_size("1K");
}

void lc80_state::lc80a(machine_config &config)
{
	lc80(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &lc80_state::lc80a_mem);
}

void lc80_state::lc80_2(machine_config &config)
{
	lc80(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &lc80_state::lc80_2_mem);
	m_ram->set_default_size("4K");
}

void lc80_state::lc80e(machine_config &config)
{
	lc80(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &lc80_state::lc80e_mem);
	m_ram->set_default_size("4K");

	// it is running almost twice as fast
	const XTAL clk = 3.3_MHz_XTAL / 2;
	m_maincpu->set_clock(clk);
	m_ctc->set_clock(clk);
	m_pio[0]->set_clock(clk);
	m_pio[1]->set_clock(clk);
}


// ROMs

ROM_START( lc80 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bm075.d202", 0x0000, 0x0400, CRC(e754ef53) SHA1(044440b13e62addbc3f6a77369cfd16f99b39752) ) // U505
	ROM_LOAD( "bm076.d203", 0x0800, 0x0400, CRC(2b544da1) SHA1(3a6cbd0c57c38eadb7055dca4b396c348567d1d5) ) // "
ROM_END

ROM_START( lc80a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc80_2716.bin", 0x0000, 0x0800, CRC(b3025934) SHA1(6fff953f0f1eee829fd774366313ab7e8053468c) ) // 2716
ROM_END

ROM_START( lc80_2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc80_2.bin", 0x0000, 0x1000, CRC(2e06d768) SHA1(d9cddaf847831e4ab21854c0f895348b7fda20b8) )
ROM_END

ROM_START( lc80e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc80e-0000-schach.rom", 0x0000, 0x1000, CRC(e3cca61d) SHA1(f2be3f2a9d3780d59657e49b3abeffb0fc13db89) )
	ROM_LOAD( "lc80e-1000-schach.rom", 0x1000, 0x1000, CRC(b0323160) SHA1(0ea019b0944736ae5b842bf9aa3537300f259b98) )
	ROM_LOAD( "lc80e-c000-schach.rom", 0x4000, 0x1000, CRC(9c858d9c) SHA1(2f7b3fd046c965185606253f6cd9372da289ca6f) )
ROM_END

} // anonymous namespace


// System Drivers

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
COMP( 1984, lc80,   0,      0,      lc80,    lc80,  lc80_state, empty_init, "VEB Mikroelektronik \"Karl Marx\" Erfurt", "Lerncomputer LC 80 (set 1)", MACHINE_SUPPORTS_SAVE )
COMP( 1984, lc80a,  lc80,   0,      lc80a,   lc80,  lc80_state, empty_init, "VEB Mikroelektronik \"Karl Marx\" Erfurt", "Lerncomputer LC 80 (set 2)", MACHINE_SUPPORTS_SAVE )
COMP( 1984, lc80e,  lc80,   0,      lc80e,   lc80,  lc80_state, empty_init, "VEB Mikroelektronik \"Karl Marx\" Erfurt", "Lerncomputer LC 80 (export)", MACHINE_SUPPORTS_SAVE )
COMP( 1991, lc80_2, lc80,   0,      lc80_2,  lc80,  lc80_state, empty_init, "hack (Eckart Buschendorf)", "Lerncomputer LC 80.2", MACHINE_SUPPORTS_SAVE )
