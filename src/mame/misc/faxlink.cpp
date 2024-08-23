// license:GPL-2.0+
// copyright-holders:flama12333
/*************************************************************************
Skeleton driver for Faxlink crane machine
Lai Pink skilltester crane machine
at pcb from the back
Faxlink Computer co ltd
u3 at892c52
u6 w27c512-457 also to stored music and plays. without it will sound random notes
U9 hm6116l-70
u10 3567 HX881 - ym2413 clone
u11 api8208 - voice rom

Direct Recording for reference https://youtu.be/uFzPDBtm-U0

No progress until the internal rom is dumped.

*/
#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ymopl.h"
#include "speaker.h"

namespace {

class faxlink_state : public driver_device
{
public:
	faxlink_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)

	{ }

	void skilltester(machine_config &config);

private:
	void io_map(address_map &map);
	void program_map(address_map &map);

protected:
	virtual void machine_start() override;

};

static INPUT_PORTS_START( skilltester )
    PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
    PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
    PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
    PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")
INPUT_PORTS_END

void faxlink_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0xffff).rom().region("eeprom", 0x2000);


}

void faxlink_state::io_map(address_map &map)
{
// No progress until the internal rom is dumped.

}

void faxlink_state::machine_start()
{
}

void faxlink_state::skilltester(machine_config &config)
{
	/* basic machine hardware */
	i8052_device &maincpu(I8052(config, "maincpu", XTAL(11'059'200)));
	maincpu.set_addrmap(AS_PROGRAM, &faxlink_state::program_map);
	maincpu.set_addrmap(AS_IO, &faxlink_state::io_map);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
    ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( skilltester )
	ROM_REGION( 0x02000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "skill_tester_ver_1_3.u3", 0x00000, 0x02000, NO_DUMP ) // actual CPU is a Atmel at892c52. protected. has the internal program code on it.
 	ROM_REGION( 0x10000, "eeprom", 0 )
     ROM_LOAD( "skilltester_program.u6", 0x00000, 0x10000, BAD_DUMP  CRC(9b9330f3) SHA1(8f6cfdbba462e6c61fa15e1cb129fadbbe27aafa) ) // 1xxxxxxxxxxxxxxx = 0xFF. label broken. left pin error at 7 and 10.

	ROM_REGION( 0x40000, "voice", 0 )
    ROM_LOAD( "api8208.bin", 0x00000, 0x40000, NO_DUMP ) // api8208 voice rom
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT   MACHINE   INPUT   STATE         INIT        ROT   COMPANY      FULLNAME                                                FLAGS
GAME( 1999, skilltester, 0, skilltester, skilltester, faxlink_state, empty_init, ROT0, "Lai Games (Faxlink)", "Pink Skilltester", MACHINE_IS_SKELETON_MECHANICAL ) // // needs MCU dump
