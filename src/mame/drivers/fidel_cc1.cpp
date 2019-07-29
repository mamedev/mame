// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Sean Riddle
/******************************************************************************

Fidelity's 1st generation chess computers:
- *Chess Challenger
- Chess Challenger 3
- *Chess Challenger 10 (UCC10)

* denotes not dumped (actually CC1 is dumped, but with half of the contents missing)

*******************************************************************************

Chess Challenger (1)
--------------------
This is the world's 1st released dedicated chess computer. Oddly, the rows/columns
are reversed: left to right is 1-8, bottom to top is A-H, eg. pawn from D2 to D4
is 4B to 4D here.

The CC1 patent(US4235442) refers to a Hewlett Packard chess program. It was eventually
found out that it was written for a HP-9810A by Alan A. Wray in 1974, and CC1 is very
similar to it. Ron C. Nelson must have ported the algorithms to 8080 when he wrote
his Altair 8800 chess program, and this is what made it into CC1.

CC1 hardware overview:
- PCB label PC-P-86, P179 C-2 7.77
- NEC 8080AF @ 2MHz(18MHz XTAL through a 8224)
- Everything goes via a NEC B8228, its special features are unused.
- NEC 2316A ROM(2KB), 4*2101AL RAM(0.5KB total)
- 8255C for I/O, 4*7seg display + 2 extra leds, 12-key keypad

Chess Challenger 3 is on the same hardware, but with double ROM size, and they
corrected the reversed chess notation. It was also offered as an upgrade to CC1.
PCB label P179 C-3 9.77.

Chess Challenger 10 version 'C'(model UCC10) is on (nearly) the same PCB too,
same label as CC3, with a small daughterboard for 8KB ROM. Again, it was also
offered as an upgrade to CC1, or CC3.

******************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "video/pwm.h"

// internal artwork
#include "fidel_cc1.lh" // clickable
#include "fidel_cc3.lh" // clickable


namespace {

class cc1_state : public driver_device
{
public:
	cc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255(*this, "ppi8255"),
		m_display(*this, "display"),
		m_delay(*this, "delay"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// RE button is tied to 8224 RESIN pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// machine drivers
	void cc1(machine_config &config);
	void cc3(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255;
	required_device<pwm_display_device> m_display;
	optional_device<timer_device> m_delay;
	required_ioport_array<2> m_inputs;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	void update_display();
	DECLARE_READ8_MEMBER(ppi_porta_r);
	DECLARE_WRITE8_MEMBER(ppi_portb_w);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);

	u8 m_led_select;
	u8 m_7seg_data;
};

void cc1_state::machine_start()
{
	// zerofill
	m_led_select = 0;
	m_7seg_data = 0;

	// register for savestates
	save_item(NAME(m_led_select));
	save_item(NAME(m_7seg_data));
}



/******************************************************************************
    I/O
******************************************************************************/

// I8255 PPI

void cc1_state::update_display()
{
	// 4 7segs + 2 leds
	m_display->matrix(m_led_select, m_7seg_data);
}

READ8_MEMBER(cc1_state::ppi_porta_r)
{
	// 74148(priority encoder) I0-I7: inputs
	// d0-d2: 74148 S0-S2, d3: 74148 GS
	u8 data = count_leading_zeros(m_inputs[0]->read()) - 24;

	// d5-d7: more inputs (direct)
	data |= ~m_inputs[1]->read() << 5 & 0xe0;

	// d4: 555 Q
	return data | ((m_delay->enabled()) ? 0x10 : 0);
}

WRITE8_MEMBER(cc1_state::ppi_portb_w)
{
	// d0-d6: digit segment data
	m_7seg_data = bitswap<7>(data,0,1,2,3,4,5,6);
	update_display();
}

WRITE8_MEMBER(cc1_state::ppi_portc_w)
{
	// d6: trigger monostable 555 (R=15K, C=1uF)
	if (~data & m_led_select & 0x40 && !m_delay->enabled())
		m_delay->adjust(attotime::from_msec(17));

	// d0-d3: digit select
	// d4: check led, d5: lose led
	m_led_select = data;
	update_display();
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8H") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7G") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6F") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5E") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4D") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3C") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2B") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1A") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, cc1_state, reset_button, nullptr)
INPUT_PORTS_END

static INPUT_PORTS_START( cc3 )
	PORT_INCLUDE( cc1 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
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
	m_ppi8255->tri_pc_callback().set_constant(0);

	TIMER(config, "delay").configure_generic(timer_device::expired_delegate());

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_cc1);
}

void cc1_state::cc3(machine_config &config)
{
	cc1(config);
	config.set_default_layout(layout_fidel_cc3);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( cc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "d2316ac_011", 0x0000, 0x0800, BAD_DUMP CRC(e27f9816) SHA1(ad9881b3bf8341829a27e86de27805fc2ccb5f7d) ) // A4 line was broken
ROM_END


ROM_START( cc3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2332c_011", 0x0000, 0x1000, CRC(51cf4682) SHA1(197374c633a0bf1a9b7ea51a72dc2b89a6c9c508) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME  PARENT CMP MACHINE  INPUT  STATE      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1977, cc1,  0,      0, cc1,     cc1,   cc1_state, empty_init, "Fidelity Electronics", "Chess Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )

CONS( 1977, cc3,  0,      0, cc3,     cc3,   cc1_state, empty_init, "Fidelity Electronics", "Chess Challenger 3", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW )
