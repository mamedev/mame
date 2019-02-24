// license:BSD-3-Clause
// copyright-holders:Kevin Horton,Jonathan Gevaryahu,Sandro Ronco,hap
/******************************************************************************
*
* fidel_vcc.cpp, subdriver of machine/fidelbase.cpp, machine/chessbase.cpp

Fidelity Voice Chess Challenger series hardware
- Voice Chess Challenger (VCC) (version A and B?)
- Advanced Voice Chess Challenger (UVC)
- *Grandmaster Voice Chess Challenger
- *Decorator Challenger (FCC)

*: not dumped yet

*******************************************************************************

RE notes by Kevin Horton

The CPU is a Z80 running at 4MHz.  The TSI chip runs at around 25KHz, using a
470K / 100pf RC network.  This system is very very basic, and is composed of just
the Z80, 4 ROMs, the TSI chip, and an 8255.

The Z80's interrupt inputs are all pulled to VCC, so no interrupts are used.

Reset is connected to a power-on reset circuit and a button on the keypad (marked RE).

The TSI chip connects to a 4K ROM.  All of the 'Voiced' Chess Challengers
use this same ROM  (three or four).  The later chess boards use a slightly different part
number, but the contents are identical.

Memory map (VCC):
-----------
0000-0FFF: 4K 2332 ROM 101-32013
1000-1FFF: 4K 2332 ROM VCC2
2000-2FFF: 4K 2332 ROM VCC3
4000-5FFF: 1K RAM (2114 SRAM x2)
6000-FFFF: empty

Memory map (UVC):
-----------
0000-1FFF: 8K 2364 ROM 101-64017
2000-2FFF: 4K 2332 ROM 101-32010
4000-5FFF: 1K RAM (2114 SRAM x2)
6000-FFFF: empty

Port map:
---------
00-03: 8255 port chip, mirrored over the 00-FF range; program accesses F4-F7

8255 connections:
-----------------
PA.0 - segment G, TSI A0 (W)
PA.1 - segment F, TSI A1 (W)
PA.2 - segment E, TSI A2 (W)
PA.3 - segment D, TSI A3 (W)
PA.4 - segment C, TSI A4 (W)
PA.5 - segment B, TSI A5 (W)
PA.6 - segment A, language latch Data (W)
PA.7 - TSI START line, language latch clock (W, see below)

PB.0 - dot commons (W)
PB.1 - NC
PB.2 - digit 0, bottom dot (W)
PB.3 - digit 1, top dot (W)
PB.4 - digit 2 (W)
PB.5 - digit 3 (W)
PB.6 - enable language switches (W, see below)
PB.7 - TSI BUSY line (R)

(button rows pulled up to 5V through 2.2K resistors)
PC.0 - button row 0, German language jumper (R)
PC.1 - button row 1, French language jumper (R)
PC.2 - button row 2, Spanish language jumper (R)
PC.3 - button row 3, special language jumper (R)
PC.4 - button column A (W)
PC.5 - button column B (W)
PC.6 - button column C (W)
PC.7 - button column D (W)

language switches:
------------------
When PB.6 is pulled low, the language switches can be read.  There are four.
They connect to the button rows.  When enabled, the row(s) will read low if
the jumper is present.  English only VCC's do not have the 367 or any pads stuffed.
The jumpers are labeled: French, German, Spanish, and special.

language latch:
---------------
There's an unstuffed 7474 on the board that connects to PA.6 and PA.7.  It allows
one to latch the state of A12 to the speech ROM.  The English version has the chip
missing, and a jumper pulling "A12" to ground.  This line is really a negative
enable.

To make the VCC multi-language, one would install the 74367 (note: it must be a 74367
or possibly a 74LS367.  A 74HC367 would not work since they rely on the input current
to keep the inputs pulled up), solder a piggybacked ROM to the existing English
speech ROM, and finally install a 7474 dual flipflop.

This way, the game can then detect which secondary language is present, and then it can
automatically select the correct ROM(s).  I have to test whether it will do automatic
determination and give you a language option on power up or something.

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "speaker.h"

// internal artwork
#include "fidel_vcc.lh" // clickable


namespace {

class vcc_state : public fidelbase_state
{
public:
	vcc_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255")
	{ }

	// machine drivers
	void vcc(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<i8255_device> m_ppi8255;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_READ8_MEMBER(speech_r);
	DECLARE_WRITE8_MEMBER(ppi_porta_w);
	DECLARE_READ8_MEMBER(ppi_portb_r);
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
};

void vcc_state::machine_start()
{
	fidelbase_state::machine_start();

	// game relies on RAM initialized filled with 1
	for (int i = 0; i < 0x400; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i + 0x4000, 0xff);
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// misc handlers

void vcc_state::prepare_display()
{
	// 4 7seg leds (note: sel d0 for extra leds)
	u8 outdata = (m_7seg_data & 0x7f) | (m_led_select << 7 & 0x80);
	set_display_segmask(0xf, 0x7f);
	display_matrix(8, 4, outdata, m_led_select >> 2 & 0xf);
}

READ8_MEMBER(vcc_state::speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}


// I8255 PPI

WRITE8_MEMBER(vcc_state::ppi_porta_w)
{
	// d0-d6: digit segment data, bits are xABCDEFG
	m_7seg_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
	prepare_display();

	// d0-d5: TSI C0-C5
	// d7: TSI START line
	m_speech->data_w(space, 0, data & 0x3f);
	m_speech->start_w(data >> 7 & 1);

	// d6: language latch data
	// d7: language latch clock (latch on high)
	if (data & 0x80)
	{
		m_speech->force_update(); // update stream to now
		m_speech_bank = data >> 6 & 1;
	}
}

READ8_MEMBER(vcc_state::ppi_portb_r)
{
	// d7: TSI BUSY line
	return (m_speech->busy_r()) ? 0x80 : 0x00;
}

WRITE8_MEMBER(vcc_state::ppi_portb_w)
{
	// d0,d2-d5: digit/led select
	// _d6: enable language switches
	m_led_select = data;
	prepare_display();
}

READ8_MEMBER(vcc_state::ppi_portc_r)
{
	// d0-d3: multiplexed inputs (active low)
	// also language switches, hardwired with 4 jumpers
	// 0(none wired): English, 1: German, 2: French, 4: Spanish, 8:Special(unused)
	u8 lan = (~m_led_select & 0x40) ? m_language : 0;
	return ~(lan | read_inputs(4)) & 0xf;
}

WRITE8_MEMBER(vcc_state::ppi_portc_w)
{
	// d4-d7: input mux (inverted)
	m_inp_mux = ~data >> 4 & 0xf;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void vcc_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x43ff).mirror(0x1c00).ram();
}

void vcc_state::main_io(address_map &map)
{
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( vcc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Speaker") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("RESET") // is not on matrix IN.0 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, vcc_state, reset_button, nullptr)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void vcc_state::vcc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vcc_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &vcc_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(vcc_state::ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set(FUNC(vcc_state::ppi_portb_r));
	m_ppi8255->out_pb_callback().set(FUNC(vcc_state::ppi_portb_w));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->in_pc_callback().set(FUNC(vcc_state::ppi_portc_r));
	m_ppi8255->out_pc_callback().set(FUNC(vcc_state::ppi_portc_w));

	TIMER(config, "display_decay").configure_periodic(FUNC(vcc_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_vcc);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->ext_read().set(FUNC(vcc_state::speech_r));
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( vcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn19256n_101-32013", 0x0000, 0x1000, CRC(257bb5ab) SHA1(f7589225bb8e5f3eac55f23e2bd526be780b38b5) )
	ROM_LOAD("cn19174n_vcc2", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0) )
	ROM_LOAD("cn19175n_vcc3", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) )
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( vccg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn19256n_101-32013", 0x0000, 0x1000, CRC(257bb5ab) SHA1(f7589225bb8e5f3eac55f23e2bd526be780b38b5) )
	ROM_LOAD("cn19174n_vcc2", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0) )
	ROM_LOAD("cn19175n_vcc3", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( vccsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn19256n_101-32013", 0x0000, 0x1000, CRC(257bb5ab) SHA1(f7589225bb8e5f3eac55f23e2bd526be780b38b5) )
	ROM_LOAD("cn19174n_vcc2", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0) )
	ROM_LOAD("cn19175n_vcc3", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) ) // dumped from Spanish VCC, is same as data in fexcelv
ROM_END

ROM_START( vccfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn19256n_101-32013", 0x0000, 0x1000, CRC(257bb5ab) SHA1(f7589225bb8e5f3eac55f23e2bd526be780b38b5) )
	ROM_LOAD("cn19174n_vcc2", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0) )
	ROM_LOAD("cn19175n_vcc3", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( uvc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64017", 0x0000, 0x2000, CRC(f1133abf) SHA1(09dd85051c4e7d364d43507c1cfea5c2d08d37f4) ) // MOS // 101-64017 // 3880
	ROM_LOAD("101-32010", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) ) // NEC P9Z021 // D2332C 228 // 101-32010, == cn19175n_vcc3 on vcc

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d) ) // NEC P9Y019 // D2332C 229 // 101-32107
	ROM_RELOAD(           0x1000, 0x1000)
ROM_END

ROM_START( uvcg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64017", 0x0000, 0x2000, CRC(f1133abf) SHA1(09dd85051c4e7d364d43507c1cfea5c2d08d37f4) )
	ROM_LOAD("101-32010", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64101", 0x0000, 0x2000, BAD_DUMP CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff) ) // taken from fexcelv, assume correct
ROM_END

ROM_START( uvcsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64017", 0x0000, 0x2000, CRC(f1133abf) SHA1(09dd85051c4e7d364d43507c1cfea5c2d08d37f4) )
	ROM_LOAD("101-32010", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9) )
ROM_END

ROM_START( uvcfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64017", 0x0000, 0x2000, CRC(f1133abf) SHA1(09dd85051c4e7d364d43507c1cfea5c2d08d37f4) )
	ROM_LOAD("101-32010", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD("101-64105", 0x0000, 0x2000, BAD_DUMP CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3) ) // taken from fexcelv, assume correct
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME   PARENT CMP MACHINE  INPUT  STATE      INIT              COMPANY, FULLNAME, FLAGS
CONS( 1979, vcc,   0,      0, vcc,     vcc,   vcc_state, init_language<0>, "Fidelity Electronics", "Voice Chess Challenger (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, vccg,  vcc,    0, vcc,     vcc,   vcc_state, init_language<1>, "Fidelity Electronics", "Voice Chess Challenger (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, vccsp, vcc,    0, vcc,     vcc,   vcc_state, init_language<4>, "Fidelity Electronics", "Voice Chess Challenger (Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, vccfr, vcc,    0, vcc,     vcc,   vcc_state, init_language<2>, "Fidelity Electronics", "Voice Chess Challenger (French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, uvc,   vcc,    0, vcc,     vcc,   vcc_state, init_language<0>, "Fidelity Electronics", "Advanced Voice Chess Challenger (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, uvcg,  vcc,    0, vcc,     vcc,   vcc_state, init_language<1>, "Fidelity Electronics", "Advanced Voice Chess Challenger (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, uvcsp, vcc,    0, vcc,     vcc,   vcc_state, init_language<4>, "Fidelity Electronics", "Advanced Voice Chess Challenger (Spanish)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1980, uvcfr, vcc,    0, vcc,     vcc,   vcc_state, init_language<2>, "Fidelity Electronics", "Advanced Voice Chess Challenger (French)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
