// license:BSD-3-Clause
// copyright-holders:
/*
  Gaelco 'Futbol-3' hardware for kiddie rides

  The PCB is very compact and has few components. The main ones are:

  PIC16C55 as main CPU
  OKI M6295 for sound
  1 bank of 6 dips

  Gaelco FUTBOL-3 PCB
  _____________________________________________________
  |JP1  JP2         __________                        |
  | __   ___        |ULN2803A_|                       |
  || |  |  |        ___________                       |
  || |  |  |        |MC74HCT273A                      |
  ||_|  |  |        _________   _________             |
  |JP3  |  |        |TLP504A_| |TLP504A_|             |
  | __  |  |        __________                        |
  || |  |  |        |PIC16C55|                        |
  ||_|  |  |                                          |
  | ___ |__|   ___ <-SN74LS365AN                      |
  | VOL        |  |   ______    ___________________   |
  |  _______   |  |  | OKI |   |ROM U1             |  |
  | |DIPSx6|   |  |  |_____|   |___________________|  |
  |            |__|                                   |
  |___________________________________________________|

  JP1 = 10 pin [+5V, GND, DAT, CLK, ENA, PU1, PU2, PU3, PU4, GND]
  JP2 = 14 pin [12VA, 12VA, +5V, ALT, CON, BOM, MOT, N/U, BOM, POT, ALT, 12V, GND, GND]
  JP3 =  5 pin [PU5, PU6, PU7, PU8, GND]

  The PCBs were inside two "Coche de Bomberos" kiddie rides from CMC Cresmatic (https://www.recreativas.org/coche-de-bomberos-6022-cresmatic).
  Anyway, the hardware is generic enough to serve any basic kiddie ride.

  The only dumped ROM on the PCB is the Oki sound samples. There are two different versions dumped (from two different machines):
     -"Susanita tiene un ratón" - Based on the song composed by Emilio Alberto Aragón 'Miliki'.
     -"El auto de papá" - Based on the song composed by Enrique Fischer 'Pipo Pescador'.
*/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "speaker.h"


class gaelcof3_state : public driver_device
{
public:
	gaelcof3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void gaelcof3(machine_config &config);

private:
	required_device<cpu_device>     m_maincpu;
};


static INPUT_PORTS_START( gaelcof3 )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // only 6 switches
	PORT_DIPNAME( 0x01, 0x01, "DSW1-1" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1-2" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1-3" )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-4" )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1-5" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1-6" )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void gaelcof3_state::gaelcof3(machine_config &config)
{
	PIC16C55(config, m_maincpu, 4000000); // clock not confirmed

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock and pin not confirmed
}


ROM_START( autopapa )
	ROM_REGION( 0x400, "maincpu", 0 )
	ROM_LOAD( "pic16c55.u3", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "autopapa.u1", 0x00000, 0x40000, CRC(a3e5607e) SHA1(24a9c79edec7b2f7f64b622240f2ad8f3ffa29ca) )
ROM_END

ROM_START( susanita )
	ROM_REGION( 0x400, "maincpu", 0 )
	ROM_LOAD( "pic16c55.u3", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "susanita.u1", 0x00000, 0x40000, CRC(766868cb) SHA1(eb42dc46b865bc448052d9d67c840e51c49ce49a) )
ROM_END

GAME( 199?, autopapa, 0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco", "El auto de papa",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, susanita, 0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco", "Susanita tiene un raton", MACHINE_IS_SKELETON_MECHANICAL )
