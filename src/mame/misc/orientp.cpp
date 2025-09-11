// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
The dump of the pcb 1 was from Soccer Santiago II 6 ball pinball. Unsure if is from or not?.

at the back pcb:
small port: bonus PCB (7 segment 4-digit)
big port: playfield sensors PCB with lots of 74HC245, leds, sensors, multiplex

Buttons
K1
K2
K3
K4

pcb 1 - From Soccer Santiago II.  Unsure  if are match?. info based of guessing. Might be not correct due to poor quality.
u1  kc8279
u17 and u21 Nec D8255-AC-2
u25 unknown winbond mcu - 8052
u32  ?? - could be hm6264alp-12
u102 kc8972
u104 ym2413
u107 u6295

EEPROM:
??? ?27c020 -  w27c020? 
u33 w27c512  boot rom.
u39 at89s51 8051 mcu for protection.

pcb 2
NREAT NEW GN-001. misspelled as GREAT-NEW?

u1 kc8279
u17 and u21 m5m82c55ap-2
u25  P8052AH mcu - 8052
u32  hm6264alp-12
u39  at89s51 - 8051 mcu for protection.
u102 kc8972
u104 ym2413
u107 u6295

EEPROM:
27C020-12.
at back.
?27c512.

pcb 3
GN-001
u1 D8279C-2
u17 and u21  D8255AC-2
u25 P8052AH mcu - 8052
u32 hm6264alp-15
u39 at89s51 - 8051 mcu for protection.
u102 kc8972
u104 ym2413
u107 u6295

EEPROM:
at27c020
at back.
winbond w27e512-12 

pcb 4 A.LC M.D.P labeled SY-OP-001
a1 EPM7032LC44-10
u1 D8279C-2
u17 and u21 NEC D8255-AC-2
u25  P8052AH mcu - 8052
u32 hm6264alp-12
u39 at89s51 8051 mcu for protection.
u100 CY CHENG YING-7 060329 - ???
u102 jfc 95101
u104 unpopulated ym2413.
u107 u6295

EEPROM:
st ????? - probably 29f1615. labeled A-0. not dumped
am27c020 labeled Mujer 1 español. dumped.
at back pcb
?27c512.

// TODO:
In which dump belong in the pcb?
MCU, Inputs. Hook up
Verify memory maps.
Need Proper Layout.

*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "machine/nvram.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "orientp.lh"

#include "speaker.h"

namespace {

class orientp_state : public driver_device
{
public:
	orientp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW1", "PUSHBUTTONS" })
	{ }

	void orientp(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	void mcu_io_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;
    void display_7seg_data_w(uint8_t data) ATTR_COLD;
	void multiplex_7seg_w(uint8_t data) ATTR_COLD;
	void ay1_port_a_w(uint8_t data) ATTR_COLD;
	void ay1_port_b_w(uint8_t data) ATTR_COLD;
	uint8_t keyboard_r();
	uint8_t m_selected_7seg_module = 0;

	output_finder<32> m_digits;
	output_finder<30> m_leds;
	required_ioport_array<4> m_inputs;

};

static INPUT_PORTS_START( east8 )
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) 
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void orientp_state::ay1_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

void orientp_state::ay1_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(data, i);
}

void orientp_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}


uint8_t orientp_state::keyboard_r()
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

void orientp_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // (7 seg display driver) Might be not correct

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void orientp_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void orientp_state::io_map(address_map &map)
{
    map(0xd800, 0xf7ff).ram().share("nvram"); // might be wrong
    map(0xf800, 0xf803).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf900, 0xf903).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfa00, 0xfa01).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xfb02, 0xfb03).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xfc40, 0xfc40).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
    map(0xfe00, 0xfe01).w("opll", FUNC(ym2413_device::write));
}

void orientp_state::mcu_map(address_map &map)
{
	map(0x0000, 0xfff).rom();
}

void orientp_state::mcu_io_map(address_map &map)
{
  
	
}

void orientp_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
}

void orientp_state::orientp(machine_config &config)
{
    // The 8279 (U1) and PSGs (U102) are clocked using the ALE output from the 8052.
	// This is normally 1/6th of the input clock frequency, but a pulse is skipped during external data memory accesses.
	// If the program accesses external data memory, sound generation and keyboard scanning will be affected.
	
    // basic machine hardware
	i8052_device &maincpu(I8052(config, "maincpu", XTAL(10'738'000)));
	maincpu.set_addrmap(AS_PROGRAM, &orientp_state::program_map);
	maincpu.set_addrmap(AS_IO, &orientp_state::io_map);
   
    NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	
	// at89s51 mcu
    i8051_device &mcu(I8051(config, "mcu", XTAL(10'738'000)));
    mcu.set_addrmap(AS_PROGRAM, &orientp_state::mcu_map);
	mcu.set_addrmap(AS_IO, &orientp_state::mcu_io_map);

	// 82C55 for leds
    I8255A(config, "ppi1");
  
	I8255A(config, "ppi2");
	
	// Keyboard & display interface. pasted from marywu. 
    i8279_device &kbdc(I8279(config, "kdc", XTAL(10'738'635) / 6));
	kbdc.out_sl_callback().set(FUNC(orientp_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(orientp_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(orientp_state::display_7seg_data_w));
	
	// Video
	config.set_default_layout(layout_orientp);
   
	// sound hardware
	SPEAKER(config, "mono").front_center();
	
	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 1.0);
	ay1.port_a_write_callback().set(FUNC(orientp_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(orientp_state::ay1_port_b_w));
	
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(10'738'000) / 6, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);  // Clock frequency & pin 7 not verified

}

ROM_START( east8v100 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // idk which pcb are from.
	ROM_LOAD( "27c512.u33", 0x00000, 0x10000,  CRC(85e28db5) SHA1(96f80a7d2214672c09b8f719cb573e77b8bac731) ) // Main program.  EAST8  v1.00 string

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000,   CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. 
 
    ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  No label  Rom. From Nreat-new GN-001 pcb. misspelled as Great-new

    ROM_END

ROM_START( east8v105 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // was from pcb 1
	ROM_LOAD( "w27c512.u33", 0x00000, 0x10000, CRC(8d3d1e91) SHA1(b80907df0878057a1ded8b56225059e06382b9d6) ) // Main program.  EAST8  v1.05 string

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, BAD_DUMP CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. From the pcb 2
 
    ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) BAD_DUMP SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  Voices Rom. From the pcb 2

    ROM_END

ROM_START( east8v105a ) 
	ROM_REGION( 0x10000, "maincpu", 0 ) // EAST8  v1.05 string. Different hex 17 hex at 7A Address. Intentional?
	ROM_LOAD( "supercopa512.u33", 0x00000, 0x10000, CRC(B9A44B3C) SHA1(dddf7cbdb1121fdbaf81a2feaf98d8c42ac5fbe5) ) // No rom label mentioned by the dumper. using filenames for now // Main program.  

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, BAD_DUMP CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. From the pcb 2
 
    ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) BAD_DUMP SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  Voices Rom. From the pcb 2

    ROM_END

ROM_START( east8v105b ) // EAST8  v1.05 string. Indentical but Different hex at 7A Address - same goes to east8v105a but different hex at 3EB2 3EB5 3EC0 Address.
	ROM_REGION( 0x10000, "maincpu", 0 ) 
	ROM_LOAD( "shenfa3musical.u33", 0x00000, 0x10000, BAD_DUMP CRC(E4348E1F) SHA1(3468109de5d66b2cdf7d3e5d09267670b14f73e2) ) // No string // No rom label mentioned by the dumper. using filenames for now. Marked as BAD_DUMP.

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, BAD_DUMP CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. From the pcb 2
 
    ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) BAD_DUMP SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  Voices Rom. From the pcb 2.

    ROM_END

ROM_START( unk6bp )
	ROM_REGION( 0x10000, "maincpu", 0 ) 
	ROM_LOAD( "eeprommusical.u33", 0x00000, 0x10000, BAD_DUMP CRC(7211acd8) SHA1(742f949c4ac661f41cf5aca42a279dc82cba2e2e) ) // No string // No rom label mentioned by the dumper. using filenames for now. Marked as BAD_DUMP.

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, BAD_DUMP CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. From the pcb 2
 
    ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) BAD_DUMP SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  Voices Rom. From the pcb 2.

    ROM_END


} // anonymous namespace


// unsorted: in which board are from?
//    YEAR  NAME            PARENT   MACHINE   INPUT  STATE   INIT        ROT   COMPANY      FULLNAME                         FLAGS
GAME( 199?, east8v100,   0,  orientp,  east8,   orientp_state, empty_init, ROT0, "<unknown>", "Unknown EAST8 (v1.00)",        MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // error 10
GAME( 199?, east8v105,   0,  orientp,  east8,   orientp_state, empty_init, ROT0, "<unknown>", "Unknown EAST8 (v1.05)",        MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // 
GAME( 199?, east8v105a,  0,  orientp,  east8,   orientp_state, empty_init, ROT0, "<unknown>", "Unknown EAST8 (v1.05) set 1",  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // 
GAME( 199?, east8v105b,  0,  orientp,  east8,   orientp_state, empty_init, ROT0, "<unknown>", "Unknown EAST8 (v1.05) set 2",  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) //
GAME( 199?, unk6bp,      0,  orientp,  east8,   orientp_state, empty_init, ROT0, "<unknown>", "Unknown 6 Ball Pinball",       MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // No String. no 7 segemnt display output. bad dump?

