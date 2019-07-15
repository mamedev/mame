// license:BSD-3-Clause
// copyright-holders:

/*
Miss Bamby - Automatics Pasqual (ClawGrip 2019-01-21)
   _____________________________________________________________
   |                             _______                        |
   |                             |__??__|      ____________     |
   |                  __________________       | EMPTY     |    |
   |                  | M5L8085AP       |      |_SOCKET____|    |
   |                  |_________________|      ____________     |
   |                       ______________      | ROM1      |    |
   |         XTAL          |M5L8212P    |      |___________|    |
   |   __   6.144          |____________|      ____________     |
   |   |R|                                     | ROM0      |    |
 __|                                           |___________|    |
|__| ________  ________  ___________________   ____________     |
|__| |ULN2003A |ULN2003A | M5L8155P         |  |_D5101LC__|     |
|__|                     |__________________|  ____________     |
|__|                     ________   _________  |_D5101LC__|     |
|__|           _______   |74LS393|  |74LS74B1                   |
|__|           |7407N |                        ________         |
|__|                     ________   ________   |GD4001B|        |
|__|                     |74LS14_|  |74LS153|        _________  |
|__|                                          ____  | BATT    | |
|__|   _______           ___________________  |D  | | 3.6V    | |
|__|   |LM380N|          |    AY-3-8910     | |I  | |_________| |
|__|                     |__________________| |P  | ______      |
|__|                                          |S__| LM311N      |
   |____________________________________________________________|
*/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
//#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "sound/ay8910.h"

class missbamby_state : public driver_device
{
public:
	missbamby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void missbamby(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void prg_map(address_map &map);

	virtual void machine_start() override;
};

uint32_t missbamby_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void missbamby_state::prg_map(address_map &map) // preliminary, everything to be taken with a grain of salt
{
	map(0x0000, 0x3fff).rom();
	map(0x5000, 0x50ff).ram();
	//map(0x6000, 0x6000).r(); // only read once at start-up?
	map(0x8000, 0x80ff).ram();
	map(0x8800, 0x88ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x8900, 0x8907).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x9000, 0x9003).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

void missbamby_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
}


static INPUT_PORTS_START( missbamby )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void missbamby_state::machine_start()
{
}



void missbamby_state::missbamby(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 6.144_MHz_XTAL); // M5L8085AP
	m_maincpu->set_addrmap(AS_PROGRAM, &missbamby_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &missbamby_state::io_map);

	PIT8253(config, "pit", 6.144_MHz_XTAL/4); // guess: only ML82 readable, might be something else

	I8155(config, "i8155", 6.144_MHz_XTAL/2); // M5L8155P, guessed divisor

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, "ay8910", 6.144_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 1.0); // guess
}


ROM_START( msbamby )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "1.bin", 0x0000, 0x2000, CRC(7b5efbd9) SHA1(abb4b4432021945aee474c4bdd83979f6460c671) )
	ROM_LOAD( "2.bin", 0x2000, 0x2000, CRC(6048d5cd) SHA1(a3bbf43b1474de75aef9957b967ead96b9a18fc5) )
ROM_END


GAME( 198?, msbamby, 0, missbamby, missbamby, missbamby_state, empty_init, ROT0, "Automatics Pasqual", "Miss Bamby", MACHINE_IS_SKELETON_MECHANICAL )
