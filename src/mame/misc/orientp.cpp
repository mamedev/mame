// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
// 


 notes: 
The dump of set 2 was from Soccer Santiago II 6 ball pinball I dont known which was from due to unconfirmed, so im gonna to name as unknown.

these roulettes use a 2x8 scan matrix for keyboard with the 8279. Only 14 buttons are used.

Port at back pcb:
small port: bonus PCB (7 segment 4-digit)
big port: playfield sensors PCB with lots of 74HC245, leds, sensors, multiplex

Hardware info - may not accurate
Buttons
K1
K2
K3
K4
pcb 1
u1 File kc8279
u17 and u21 Nec D8255-AC-2
u25  winbond unknown - 8052
u32  ?? - could be hm6264alp-12
u102 kc8972
u104 ym2413
u107 u6295



EEPROM:
??  winbond w27c020? cant read text due to poor pictures quality
u33 winbond w27c512  boot rom.
u39 at89s51 8051 mcu for protection.

pcb 2 A.LC M.D.P
a1 unknown pld
u1 Nec D8279C-2
u17 and u21 Nec D8255-AC-2
u25  Intel P8052AH mcu - 8052
u32  hm6264alp-12
u102 kc8972
u104 ym2413
u107 u6295





// TODO:
Need proper description.
Error code 10
Hardware info.
MCU Hook up
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) // K0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) // K1
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) // K2
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) // K3
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
	
	//TODO: Check mcu dump and hook up
    i8051_device &mcu(I8051(config, "mcu", XTAL(10'738'000)));
    mcu.set_addrmap(AS_PROGRAM, &orientp_state::mcu_map);
	mcu.set_addrmap(AS_IO, &orientp_state::mcu_io_map);

	//TODO: M82C55 for leds
    I8255A(config, "ppi1");
    I8255A(config, "ppi2");
	
	// Keyboard & display interface 
	//TODO: pasted from marywu
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

ROM_START( east8 )
	ROM_REGION( 0x10000, "maincpu", 0 ) 
	ROM_LOAD( "27c512.u33", 0x00000, 0x10000,  CRC(85e28db5) SHA1(96f80a7d2214672c09b8f719cb573e77b8bac731) ) // Main program.  EAST8  v1.00 string

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000,   CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. 
 
    ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  No label  Rom. From Nreat-new GN-001 pcb. misspelled as Great-new?

    ROM_END

ROM_START( east8a )
	ROM_REGION( 0x10000, "maincpu", 0 ) 
	ROM_LOAD( "w27c512.u33", 0x00000, 0x10000, CRC(8d3d1e91) SHA1(b80907df0878057a1ded8b56225059e06382b9d6) ) // Main program.  EAST8  v1.05 string

    ROM_REGION( 0x1000, "mcu", 0 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, BAD_DUMP CRC(a55b63a8) SHA1(9ef88bba4a46ccd969d80882e9c36eb2f0c9e4bf) ) //  Microcontroller Protection. From the Nreat-new GN-001 pcb. Marked as Bad dump for now
 
    ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, CRC(f962ed1c) BAD_DUMP SHA1(c69cd9619c794e77a0122fc82d36662494ceb0be) ) //  Voices Rom.  From Nreat-new GN-001 pcb. Marked as Bad dump for now

    ROM_END


} // anonymous namespace
//    YEAR  NAME    PARENT   MACHINE   INPUT  STATE          INIT        ROT   COMPANY      FULLNAME                        FLAGS
GAME( 199?, east8,  0,       orientp,  east8, orientp_state, empty_init, ROT0, "<unknown>", "Unknown EAST8 (v1.00)",  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, east8a, 0,       orientp,  east8, orientp_state, empty_init, ROT0, "<unknown>", "Unknown EAST8 (v1.05)",  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
