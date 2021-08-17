// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for "Basket". Electromechanical machine from the 
Spanish company IGOA S.A.
   ____________________________________________________
  |                          _________                 |
  |                         | 8xDips  |                |__
  |  _____________   _________________                  __|
  | | EMPTY       | | GI AY-3-8910A   |                 __|
  | |_____________| |_________________|                 __|
  |  _____________   _________________                  __|
  | | EMPTY       | | NEC D8155HC     |                 __|
  | |_____________| |_________________|                 __|                
  |  _____________   _______  _________      ________   __|
  | | EPROM       | |DM7417N  SN74LS373N    |ULN2003R|  __|
  | |_____________|  _________________       ________   __|
  |   ____________  | M5L8085AP       |     |ULN2003A|  __|
  |  | HM6116LP-3 | |_________________|                |
  |  |____________|           ________                 |__
  |   ________         Xtal   SN74LS14N                 __|
  |   82S123AN     6.144 MHz  _________________         __|
  |   ________               | NEC D8155HC     |        __|
  |   74LS393N               |_________________|        __|
  |   ________    ________                              __|
  |  PAL16R4ACN  |TC4011BP|                             __|
  |                  _____                              __|
  |  __________     NE555N             ________         __|
  |  | BATT 5V |                      |ULN2803A         __|
  |  |_________|                                        __|
  |     __                                             |
  |    (__) <- Button                                  |
  |____________________________________________________|

*/

#include "emu.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "machine/i8155.h"
#include "sound/ay8910.h"

namespace {

class igoabasket_state : public driver_device
{
public:
	igoabasket_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void igoabasket(machine_config &config);
	void gldwinner(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void io_map(address_map &map);
	void prg_map(address_map &map);

	virtual void machine_start() override;
};

void igoabasket_state::prg_map(address_map &map)
{
}

void igoabasket_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( igoabasket )
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

void igoabasket_state::machine_start()
{
}

void igoabasket_state::igoabasket(machine_config &config)
{
	Z80(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igoabasket_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &igoabasket_state::io_map);

	I8155(config, "i8155a", 6.144_MHz_XTAL/2); // Guessed divisor
	I8155(config, "i8155b", 6.144_MHz_XTAL/2); // Guessed divisor

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 6.144_MHz_XTAL / 4)); // Guessed divisor
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

// Two empty EPROM sockets on the dumped PCB
ROM_START( basket )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "igoa_sa_ba2.81_chk_69d2_mod_basket.bin", 0x0000, 0x8000, CRC(3d52419d) SHA1(5f097391bcad72c8d0f029ef21ca38d903cef140) )

	ROM_REGION(0x20, "prom", 0)
	ROM_LOAD( "82s123an.bin", 0x00, 0x20, NO_DUMP )

	ROM_REGION(0x104, "plds", 0)
	ROM_LOAD( "pal16r4acn.bin", 0x000, 0x104, NO_DUMP )
ROM_END

} // Anonymous namespace

GAME( 1987?, basket, 0, igoabasket, igoabasket, igoabasket_state, empty_init, ROT0, "Igoa S.A.", "Basket", MACHINE_IS_SKELETON_MECHANICAL ) // v2.0 on ROM string, v2.81 on EPROM label
