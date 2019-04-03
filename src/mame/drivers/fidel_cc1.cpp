// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Sean Riddle
/******************************************************************************
*
* fidel_cc1.cpp, subdriver of machine/fidelbase.cpp, machine/chessbase.cpp

Fidelity's 1st generation chess computers:
- *Chess Challenger
- *Chess Challenger 3
- *Chess Challenger 10 (UCC10)

* denotes not dumped (actually CC1 is dumped, but with half of the contents missing)

TODO:
- driver is untested, but it should be easy to get working when a good dump
  shows up (or a rom of CC3, even)

*******************************************************************************

Chess Challenger (1)
--------------------
This is the world's 1st released dedicated chess computer. Oddly, the rows/columns
are reversed: left to right is 1-8, bottom to top is A-H, eg. pawn from D2 to D4
is 4B to 4D here.

PCB label PC-P-86, P179 C-2 7.77
NEC 8080AF @ 2MHz(18MHz XTAL through a 8224)
Everything goes via a NEC B8228, its special features are unused.
NEC 2316A ROM(2KB), 4*2101AL RAM(0.5KB total)
8255C for I/O, 4*7seg display + 2 extra leds, 12-key keypad

Chess Challenger 3 is on the same hardware, but with double ROM size, and they
corrected the reversed chess notation. It was also offered as an upgrade to CC1.

Chess Challenger 10 version 'C'(model UCC10) is on (nearly) the same PCB too,
label P179 C-3 9.77, with a small daughterboard for 8KB ROM. Again, it was also
offered as an upgrade to CC1, or CC3.

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"

// internal artwork
#include "fidel_cc1.lh" // clickable


namespace {

class cc1_state : public fidelbase_state
{
public:
	cc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255"),
		m_delay(*this, "delay")
	{ }

	// machine drivers
	void cc1(machine_config &config);

private:
	// devices/pointers
	required_device<i8255_device> m_ppi8255;
	optional_device<timer_device> m_delay;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_READ8_MEMBER(ppi_porta_r);
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// misc handlers

void cc1_state::prepare_display()
{
	// 4 7segs + 2 leds
	set_display_segmask(0xf, 0x7f);
	display_matrix(7, 6, m_7seg_data, m_led_select);
}


// I8255 PPI

READ8_MEMBER(cc1_state::ppi_porta_r)
{
	// 74148(priority encoder) I0-I7: inputs
	// d0-d2: 74148 S0-S2, d3: 74148 GS
	u8 data = count_leading_zeros(m_inp_matrix[0]->read()) - 24;

	// d5-d7: more inputs (direct)
	data |= ~m_inp_matrix[1]->read() << 5 & 0xe0;

	// d4: 555 Q
	return data | ((m_delay->enabled()) ? 0x10 : 0);
}

WRITE8_MEMBER(cc1_state::ppi_portb_w)
{
	// d0-d6: digit segment data
	m_7seg_data = bitswap<7>(data,0,1,2,3,4,5,6);
	prepare_display();
}

WRITE8_MEMBER(cc1_state::ppi_portc_w)
{
	// d6: trigger monostable 555 (R=15K, C=1uF)
	if (~data & m_led_select & 0x40 && !m_delay->enabled())
		m_delay->adjust(attotime::from_msec(17));

	// d0-d3: digit select
	// d4: check led, d5: lose led
	m_led_select = data;
	prepare_display();
}



/******************************************************************************
    Address Maps
******************************************************************************/

void cc1_state::main_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x11ff).mirror(0x0e00).ram();
}

void cc1_state::main_io(address_map &map)
{
	map.global_mask(0x0f);
	map(0x00, 0x03).mirror(0x04).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( cc1 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, cc1_state, reset_button, nullptr)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void cc1_state::cc1(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 18_MHz_XTAL/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &cc1_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &cc1_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->in_pa_callback().set(FUNC(cc1_state::ppi_porta_r));
	m_ppi8255->out_pb_callback().set(FUNC(cc1_state::ppi_portb_w));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->out_pc_callback().set(FUNC(cc1_state::ppi_portc_w));

	TIMER(config, "delay").configure_generic(timer_device::expired_delegate());

	TIMER(config, "display_decay").configure_periodic(FUNC(cc1_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_cc1);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( cc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "d2316ac_011", 0x0000, 0x0800, BAD_DUMP CRC(e27f9816) SHA1(ad9881b3bf8341829a27e86de27805fc2ccb5f7d) ) // A4 line was broken
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1977, cc1,  0,      0, cc1,     cc1,   cc1_state, empty_init, "Fidelity Electronics", "Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
