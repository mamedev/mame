// license:GPL-2.0+
// copyright-holders:flama12333

/*************************************************************************
This Driver is for Oriental Pearl hardware and clones.

Probably created by Chang yu Electronic due to same sound system like changyu2 and gluck2.

Buttons
K1
K2
K3
K4

56 leds
 5 2x 7 segment display, 
 4x 7 segment big display bonus connected in on the back of pcb.

ic
u1 kc8279
u17 and u21 nec d8255ac-2
u32 hm6264
??  winbond w27c020 adpcm rom.
u33 winbond w27c512 boot rom.
u39 at89s51 second mcu for protection.

// TODO:
Need Proper hardware info.
Hook up nvram inputs opll and adpcm.
east8 Has undumped mcu and adpcm rom.
Need Layout as and Add segment display as marywu.cpp.


Features Notes:
src: Chang yu website

Pearl in the  East 

5 balls per game ... win a bonus if 4 balls flip into the same slot.

When balls flip into certain slots, additional lamps illuminate.

When scoring on all letters of pearl in the east, up to 4 extra lamps randomly illuminate.

Higher bets when scoring on all letters of PEARL IN THE EAST win a higher bonus.

New feature: buy a 6th ball toup the odds.

Oriental Pearl - 1997

5 balls per game and 2 win modes.

1 higher bets mean more bonus lights and more chances to win.

Big bonus for scoring on all letters of "ORIENTAL PEARL".

Electronic ball-checking device ensures where ball lands.

There are clones that use music adpcm instead of ym2413.
also diferent adpcm voices.
*/

#include "emu.h"


#include "cpu/mcs51/mcs51.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "speaker.h"

#include "orientpearl.lh"


namespace {

class orientalpearl_state : public driver_device
{
public:
	orientalpearl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW1", "PUSHBUTTONS" })
		
	{ }

	void east8(machine_config &config);
protected:
	virtual void machine_start() override;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	uint8_t keyboard_r();
	void io_map(address_map &map);
	void program_map(address_map &map);

	uint8_t m_selected_7seg_module = 0;

	output_finder<32> m_digits;
	output_finder<30> m_leds;
	required_ioport_array<4> m_inputs;
};

static INPUT_PORTS_START( orientalpearl )
	PORT_START("KEYS1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)

	PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW1:8")
	
	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW2:8")
	
	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) // K0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) // K1
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) // K2
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) // K3
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void orientalpearl_state::ay1_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

void orientalpearl_state::ay1_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(data, i);
}

void orientalpearl_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}


uint8_t orientalpearl_state::keyboard_r()
{
	switch (m_selected_7seg_module & 0x07)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_inputs[m_selected_7seg_module & 0x07]->read();
	default:
		return 0x00;
	}
}

void orientalpearl_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // Might be not correct

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void orientalpearl_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void orientalpearl_state::io_map(address_map &map)
{
    map(0xfa00, 0xfa01).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
    map(0xfb02, 0xfb03).w("ay1", FUNC(ay8910_device::address_data_w));

}

void orientalpearl_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
}

void orientalpearl_state::east8(machine_config &config)
{

	/* basic machine hardware */
	i8052_device &maincpu(I8052(config, "maincpu", XTAL(10'738'000)));
	maincpu.set_addrmap(AS_PROGRAM, &orientalpearl_state::program_map);
	maincpu.set_addrmap(AS_IO, &orientalpearl_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(10'738'635) / 6));
	kbdc.out_sl_callback().set(FUNC(orientalpearl_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(orientalpearl_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(orientalpearl_state::display_7seg_data_w));

	/* Video */
	config.set_default_layout(layout_orientpearl);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay1.port_a_write_callback().set(FUNC(orientalpearl_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(orientalpearl_state::ay1_port_b_w));
	
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);
	
	OKIM6295(config, "oki", XTAL(10'738'000) / 6, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);  // Clock frequency & pin 7 not verified

}

ROM_START( east8 )
	ROM_REGION( 0x10000, "maincpu", 0 ) //  EAST8  v1.05 string
	ROM_LOAD( "w27c512.u33", 0x00000, 0x10000, CRC(8d3d1e91) SHA1(b80907df0878057a1ded8b56225059e06382b9d6) ) // main program
	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00 )
	ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, NO_DUMP ) // mcu. protection
	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, NO_DUMP ) //  oki rom voice
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   MACHINE   INPUT         STATE                INIT        ROT    COMPANY      FULLNAME                                                FLAGS
GAME( 200?, east8,  0,       east8,   orientalpearl, orientalpearl_state, empty_init, ROT0, "<unknown>", "Unknown 6 Ball Pinball Gambling", MACHINE_IS_SKELETON_MECHANICAL ) // EAST8  v1.05  string . this was dumped from soccer santiago II 6 ball pinball.
