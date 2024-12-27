// license:BSD-3-Clause
// copyright-holders:
/*
  Gaelco 'Futbol-3' hardware for kiddie rides

  The PCB is very compact and has few components. The main ones are:

  PIC16C56 as main CPU
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
  || |  |  |        |PIC16C56|                        |
  ||_|  |  |                                          |
  | ___ |__|   ___ <-SN74LS365AN                      |
  | VOL        |  |   ______    ___________________   |
  |  _______   |  |  | OKI |   |ROM U1             |  |
  | |DIPSx6|   |  |  |6295_|   |___________________|  |
  |            |__|                                   |
  |___________________________________________________|

  JP1 = 10 pin [+5V, GND, DAT, CLK, ENA, PU1, PU2, PU3, PU4, GND]
  JP2 = 14 pin [12VA, 12VA, +5V, ALT, CON, BOM, MOT, N/U, BOM, POT, ALT, 12V, GND, GND]
  JP3 =  5 pin [PU5, PU6, PU7, PU8, GND]

  The PCBs were inside two "Coche de Bomberos" kiddie rides from CMC Cresmatic (https://www.recreativas.org/coche-de-bomberos-6022-cresmatic).
  Anyway, the hardware is generic enough to serve any basic kiddie ride.

  There are three different versions dumped (from different machines):
     -"Susanita" - Based on the song composed by Rafael Pérez Botija.
     -"El auto feo" - Based on the song composed by Enrique Fischer 'Pipo Pescador'.
     -"Hola Don Pepito" - Based on the song composed by Ramón del Rivero.

  The PIC16C56 from Hola Don Pepito has been decapped. It is believed it has the same contents for all games.

  TODO:
  inputs;
  Oki hook up is possibly more complex.
*/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "speaker.h"


namespace {

class gaelcof3_state : public driver_device
{
public:
	gaelcof3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void gaelcof3(machine_config &config);

private:
	required_device<pic16c56_device> m_maincpu;
};


static INPUT_PORTS_START( gaelcof3 ) // defined as buttons only for easier testing
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("DSW1") // only 6 switches
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x30, "25 Pesetas" )
	PORT_DIPSETTING(    0x20, "100 Pesetas" )
	PORT_DIPSETTING(    0x10, "50 Pesetas" )
	PORT_DIPSETTING(    0x00, "200 Pesetas" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(    0x0e, "42 seconds" )
	PORT_DIPSETTING(    0x0c, "56 seconds" )
	PORT_DIPSETTING(    0x0a, "70 seconds" )
	PORT_DIPSETTING(    0x08, "84 seconds" )
	PORT_DIPSETTING(    0x06, "98 seconds" )
	PORT_DIPSETTING(    0x04, "112 seconds" )
	PORT_DIPSETTING(    0x02, "126 seconds" )
	PORT_DIPSETTING(    0x00, "140 seconds" )
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 )
INPUT_PORTS_END


void gaelcof3_state::gaelcof3(machine_config &config)
{
	PIC16C56(config, m_maincpu, 4000000); // clock not confirmed
	m_maincpu->read_a().set_ioport("IN0");
	m_maincpu->write_a().set([this] (uint8_t data) { logerror("port A write: %02x\n", data); });
	m_maincpu->read_b().set_ioport("DSW1");
	m_maincpu->write_b().set("oki", FUNC(okim6295_device::write));

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock and pin not confirmed
}


ROM_START( autopapa )
	ROM_REGION( 0x2000, "maincpu", 0 )
	// this was decapped and dumped for donpepito, should be the same but marking it as bad dump for overcautiousness
	ROM_LOAD( "pic16c56.u3", 0x0000, 0x1fff, BAD_DUMP CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "autopapa.u1", 0x00000, 0x40000, CRC(a3e5607e) SHA1(24a9c79edec7b2f7f64b622240f2ad8f3ffa29ca) )
ROM_END

ROM_START( donpepito )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "don_pepito.u1", 0x00000, 0x40000, CRC(574fcd14) SHA1(a23f1eb6d2cef5aa07df3a553fe1d33803648f43) )
ROM_END

ROM_START( susanita )
	ROM_REGION( 0x2000, "maincpu", 0 )
	// this was decapped and dumped for donpepito, should be the same but marking it as bad dump for overcautiousness
	ROM_LOAD( "pic16c56.u3", 0x0000, 0x1fff, BAD_DUMP CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "susanita.u1", 0x00000, 0x40000, CRC(766868cb) SHA1(eb42dc46b865bc448052d9d67c840e51c49ce49a) )
ROM_END

} // anonymous namespace


GAME( 199?, autopapa,  0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco", "El auto feo",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, donpepito, 0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco", "Hola Don Pepito", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, susanita,  0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco", "Susanita",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
