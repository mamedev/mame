// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Sega Mega Drive/Genesis-based bootlegs

    Games supported:
        * Top Shooter

Top Shooter PCB info
====================

 Sun Mixing board, looks like a hacked up Genesis clone.

 Original driver by David Haywood
 Inputs by Mariusz Wojcieszek

 Top Shooter - (c)1995  - older board, look more like an actual hacked cart system, has an MCU

TOP SHOOTER - Sun Mixing Co. Ltd. 1995

To me it seems like an original cartridge-based arcade board
hacked to use an external ROM board and a standard JAMMA
connector, but of course, I can be wrong.


   UPPER BOARD

   _________________________________________________________
   |            ___________  ___________  _____      __    |
   | 74LS245P  |U14 Empty | |U12 ROM1  |  |IC1|      |B|   |
   | 74LS245P  |__________| |__________|  |___|            |
   | 74LS245P   ___________  ___________    _____________  |
 __|           |U13 Empty | |U11 ROM2  |   | AT89C51    |  |
 |_ J          |__________| |__________|   |____________|  |_
 |_ A           ______________________              _____  |_ J
 |_ M          | U10 MC68000P10       |             |OSC|  |_ P
 |_ M          | Motorola             |                    |_ 2
 |_ A          |______________________|            74HC00P |_
 |_  74LS245P   ______________________           ________  |
 |_            | U9 Empty             |          |HM6116L  |
 |_            |                      |          |_______| |_ J
 |_            |______________________|                    |_ P
 |_  74LS245P                           TD62oo3AP 74LS373P |_ 3
 |_                                            __________  |
 |_  74LS245P                                  |GALv20V8B| |
 |_                                    ______              |
 |_               _____                |DIPS|              |_ P
   |             |U24  |                                   |_ 1
   | 74LS245P                                              |
   | TD62oo3AP                                             |
   |                                                       |
   |_            97              ____________         _____|
     |_|_|_|_|_|_|_|_|_|_|_|_|_|_|           |_|_|_|_|


  IC1 = Surface scratched out, don't know what it is
  U24 = Surface scratched out, seems like a PROM
 DIPs = Fixed as: 00001000
 ROMs = Toshiba TC574000AD

  JP2, JP3 and P1 connects both boards, also another
  on-board connector is used, see notes for the 68K socket
  for the lower board.


   LOWER BOARD

   _________________________________________________________
   |                                     ____ ____         |
   |  ___                                | I| | I|         |
   |  |I|                                | C| | C|         |
   |  |C|                                | 3| | 2|         |
   |  |1|                                |__| |__|         |
   |  |3|                                                  |__
   |   _                _________________________           __|
   |  |_|               |||||||||||||||||||||||||           __|
   |  IC14              ---------- SLOT ---------           __|
   |               ______________________                   __|
   |              |                      |                  __|
   |  ___         | 68K (to upper board) |   _______        __|
   |  |I|         |______________________|   |SE-94|        __|
   |  |C|                                    |JDDB |      _|
   |  |1|           _______                  |_____|      |
   |  |2|           |SE-93|                    IC4        |
   |                |JDDA |                               |
   |                |_____|                ___________    |_
   |                  IC8                  |Z8400A PS|     |
   |                                       |_________|     |
   |                  ______         _________  _________  |
   |                  | OSC|         | IC11  |  | IC7   |  |
   |            _____________        |_______|  |_______|  |
   |    RST    |            |           CN5        CN6     |
   |___________|            |______________________________|


   IC3 = IC2 = Winbond W24257V
   IC7  = 6264LD 9440
   IC11 = SE-95 JDDC
   IC12 = Sony CXA1634P
   IC13 = Sony CXA1145P
   IC14 = GL358 N16

   RST is a reset button.

   OSC = 53.693175 MHz

   CN5 and CN6 are 9-pin connectors... serial ports?

   There are two wires soldered directly to two connectors
   of the slot, going to the upper board (via P1).

   The whole upper board is plugged using the 68000 socket,
   there is no 68K on the lower board.

   There is an edge connector, but it isn't JAMMA.

   "HK-986 (KINYO)" is written on the PCB, near the slot.

****************************************************************************/

#include "emu.h"
#include "megadriv.h"


namespace {

class md_sunmixing_state : public md_base_state
{
public:
	md_sunmixing_state(machine_config const &mconfig, device_type type, char const *tag) :
		md_base_state(mconfig, type, tag)
	{
	}

	void topshoot(machine_config &config);
	void sbubsm(machine_config &config);

private:
	void topshoot_68k_map(address_map &map) ATTR_COLD;
	void sbubsm_68k_map(address_map &map) ATTR_COLD;

	uint16_t topshoot_200051_r();
	uint16_t sbubsm_400000_r();
	uint16_t sbubsm_400002_r();
};


/*************************************
 *
 *  Machine Configuration
 *
 *************************************/

void md_sunmixing_state::topshoot(machine_config &config)
{
	md_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_sunmixing_state::topshoot_68k_map);
}

void md_sunmixing_state::sbubsm(machine_config &config)
{
	md_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_sunmixing_state::sbubsm_68k_map);
}


void md_sunmixing_state::topshoot_68k_map(address_map &map)
{
	megadriv_68k_base_map(map);

	map(0x000000, 0x1fffff).rom(); // Cartridge Program ROM
	map(0x200000, 0x2023ff).ram(); // Tested

	// these are shared RAM, MCU puts the inputs here
	map(0x200042, 0x200043).portr("IN0");
	map(0x200044, 0x200045).portr("IN1");
	map(0x200046, 0x200047).portr("IN2");
	map(0x200048, 0x200049).portr("IN3");
	map(0x200050, 0x200051).r(FUNC(md_sunmixing_state::topshoot_200051_r));
}

void md_sunmixing_state::sbubsm_68k_map(address_map &map)
{
	topshoot_68k_map(map);

	// these are shared RAM, MCU puts the inputs here
	map(0x20007e, 0x20007f).portr("IN4");

	// needed to boot, somme kind of hardware ident?
	map(0x400000, 0x400001).r(FUNC(md_sunmixing_state::sbubsm_400000_r));
	map(0x400002, 0x400003).r(FUNC(md_sunmixing_state::sbubsm_400002_r));
}


/*************************************
 *
 *  Games memory handlers
 *
 *************************************/

uint16_t md_sunmixing_state::topshoot_200051_r()
{
	return -0x5b;
}

uint16_t md_sunmixing_state::sbubsm_400000_r()
{
	logerror("%s: sbubsm_400000_r\n", machine().describe_context().c_str());
	return 0x5500;
}

uint16_t md_sunmixing_state::sbubsm_400002_r()
{
	logerror("%s: sbubsm_400002_r\n", machine().describe_context().c_str());
	return 0x0f00;
}


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( topshoot ) // Top Shooter Input Ports
	PORT_START("IN0")
	PORT_BIT( 0x4f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet") PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Start") PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Fire") PORT_IMPULSE(1)

	PORT_START("IN1")
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Test mode down") PORT_IMPULSE(1)

	PORT_START("IN2")
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sbubsm )
	// the bit ordering in the ports is strange here because this is being read through shared RAM, the MCU presumably reads the real inputs then scrambles them in RAM for the 68k to sort out
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// no service mode here?
INPUT_PORTS_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( topshoot ) // Top Shooter (c)1995 Sun Mixing
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tc574000ad_u11_2.bin", 0x000000, 0x080000, CRC(b235c4d9) SHA1(fbb308a5f6e769f3277824cb6a3b50c308969ac2) )
	ROM_LOAD16_BYTE( "tc574000ad_u12_1.bin", 0x000001, 0x080000, CRC(e826f6ad) SHA1(23ec8bb608f954d3b915f061e7076c0c63b8259e) )

	// Not hooked up yet
	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "89c51.bin", 0x0000, 0x1000, CRC(595475c8) SHA1(8313819ba06cc92b54f88c1ca9f34be8d1ec94d0) )
ROM_END

ROM_START( sbubsm )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u11.bin", 0x000000, 0x080000, CRC(4f9337ea) SHA1(b245eb615f80afd25e29b2efdddb7f61c1deff6b) )
	ROM_LOAD16_BYTE( "u12.bin", 0x000001, 0x080000, CRC(f5374835) SHA1(3a97910f5f7327ec7ad6425dfdfa72c86196ed33) )

	ROM_REGION( 0x1000, "mcu", 0 ) // could be the same as topshoot (same PCB)
	ROM_LOAD( "89c51.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1995, topshoot, 0, topshoot, topshoot, md_sunmixing_state, init_megadriv, ROT0, "Sun Mixing", "Top Shooter",                                                 0 )
GAME( 1996, sbubsm,   0, sbubsm,   sbubsm,   md_sunmixing_state, init_megadriv, ROT0, "Sun Mixing", "Super Bubble Bobble (Sun Mixing, Mega Drive clone hardware)", 0 )
