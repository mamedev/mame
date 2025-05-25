// license::BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************

known undumped games

W&P CATCHER - lucky dog 
https://youtu.be/e1Gp7Cr6KIQ

Fantasy World - Diferent version.
https://youtu.be/I7l6sOW40gE
seem to be used in lucky loco
https://youtu.be/vCyPViEF5pk
// There are 4 langauge voices which are not configurable.

chinese:
https://youtu.be/RNM4aPQNWhg

spanish:
https://youtu.be/na70qV8awqE

english female:
https://youtu.be/wHEyHoKZkJk

english male:
https://youtu.be/aArV8y5TuzM?t=13

amusements 
https://youtu.be/qUiQgHpiAyI
*/
#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "speaker.h"

namespace {

class toysoldier_state : public driver_device
{
public:
	toysoldier_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		
	{ }

	void toysoldier(machine_config &config);

private:
	void ay_port_a_w(uint8_t data);
	void ay_port_b_w(uint8_t data);
	void io_map(address_map &map);
	void program_map(address_map &map);

protected:
	virtual void machine_start() override;

};

static INPUT_PORTS_START( toysoldier )
   
PORT_START("IN1") //  Claw input 
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
 	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
  
PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
    PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) 
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)

PORT_START("IN3") // coin
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

 
PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")


PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END

void toysoldier_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void toysoldier_state::io_map(address_map &map)
{
	map(0x8000, 0x8001).w("opll", FUNC(ym2413_device::write));
	map(0x9000, 0x9001).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
    map(0xa000, 0xa000).portr("IN1"); //  claw up claw down return input
//	map(0xa000, 0xa000).nopw(); // ???
//  map(0xb000, 0xb000).nopw(); //  aplus ic
    map(0xb000, 0xb000).portr("IN2"); // joystick
//	map(0xc000, 0xc000).nopw(); // ???
    map(0xc000, 0xc000).portr("IN3");  // Coin
//  map(0xd000, 0xd000).unmapw(); // 7 segment display  control
    map(0xe000, 0xe000).nopw(); // Super Card
}

void toysoldier_state::machine_start()
{
}

void toysoldier_state::toysoldier(machine_config &config)
{
	/* basic machine hardware */
	i8052_device &maincpu(I80C32(config, "maincpu", XTAL(12'000'000))); // Actually cpu is a Winbond W78C32c-40
	maincpu.set_addrmap(AS_PROGRAM, &toysoldier_state::program_map);
	maincpu.set_addrmap(AS_IO, &toysoldier_state::io_map);
	
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	
    ay8910_device &ay(AY8910(config, "ay", XTAL(12'000'000) / 6));  // It’s clocked by the ALE output of the 8032 – it will be 1/6 of the 8032 clock frequency, with the issue that data external memory accesses cause it to drop pulses.
    ay.add_route(ALL_OUTPUTS, "mono", 1.0);
	ay.port_a_read_callback().set_ioport("DSW1");
	ay.port_b_read_callback().set_ioport("DSW2");
	
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( toysoldier )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.u3", 0x00000, 0x10000, CRC(BD52C1AE) SHA1(a76c10f93f9087bc2d01bfe866c0e66a006b4ddd) )
	ROM_REGION( 0x40000, "voice", 0 )
    ROM_LOAD( "api8108a.u8", 0x00000, 0x40000, NO_DUMP ) // api8108a voice rom
	ROM_REGION( 0x200, "eeprom", 0 ) // according to diagram
	ROM_LOAD( "93c66.u39", 0x000, 0x200, NO_DUMP ) // probably for storing internal settings.
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT   MACHINE   INPUT   STATE         INIT        ROT   COMPANY      FULLNAME                                                FLAGS
GAME( 2003, toysoldier, 0, toysoldier, toysoldier, toysoldier_state, empty_init, ROT0, "Paokai", "Toy Soldier (Coastal Amusements)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // This was dumped from an Coastal branded Toy Soldier from around 2003.
