// license:BSD-3-Clause
// copyright-holders:
/*
  Gaelco 'Futbol-3' hardware for kiddie rides, pinballs, and electromechanicals.

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

  There is a newer version of the PCB with the same components (Gaelco REF.920505, from 1992). It adds a fuse, a LED for PCB control, and
  better connectors.

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

// Pinballs

ROM_START( futbol )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "p4n_pic16c56.bin", 0x0000, 0x2000, CRC(a4d69b51) SHA1(aa0f20b45aa92912ab235c9dc30b3532ff7103eb) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pinball_futbol_p4_97e7_p4n_26-6-98_27c020.bin", 0x00000, 0x40000, CRC(448d244b) SHA1(51c3d6309b487d17085aac161016190249e2900b) )
ROM_END

ROM_START( futbola )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "p4n_pic16c56.bin", 0x0000, 0x2000, CRC(a4d69b51) SHA1(aa0f20b45aa92912ab235c9dc30b3532ff7103eb) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pinball_futbol_p3_20f6_p4n_21-10-97_27c020.bin", 0x00000, 0x40000, CRC(05a3595d) SHA1(226fd63ea23d06022bbad9eb5a60fe04707a8fca) )
ROM_END

ROM_START( futbolt )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "test_pic16c54a.bin", 0x0000, 0x2000, CRC(ad819aaa) SHA1(f10500e9147c703e24a26b4d48305c16996b7c0a) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "test_futbol_27c010a.bin", 0x00000, 0x20000, CRC(57cf1ca4) SHA1(8d7f027bf7809194035c5b4671919d3b3dce2f1b) )
ROM_END

// Kiddie rides

// Based on the song "El auto feo", composed by Enrique Fischer 'Pipo Pescador'.
ROM_START( autopapa )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x2000, CRC(089699f5) SHA1(2cc470a97936887804363c8783bad4db4cad4f64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "autopapa.u1", 0x00000, 0x40000, CRC(a3e5607e) SHA1(24a9c79edec7b2f7f64b622240f2ad8f3ffa29ca) )
ROM_END

// Based on the song "Hola Don Pepito", composed by Ramón del Rivero.
ROM_START( donpepito )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "ir_pic16c56.u3", 0x0000, 0x1fff, CRC(a2c24ec3) SHA1(e87520c6de714b1638c9b156411522e0209fb06e) ) // Decapped

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "don_pepito.u1", 0x00000, 0x40000, CRC(574fcd14) SHA1(a23f1eb6d2cef5aa07df3a553fe1d33803648f43) )
ROM_END

// Based on the Spanish cover version of the song "I Like To Move It" by Reel 2 Real, named "Te Gusta el Mueve Mueve".
ROM_START( mueve )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x2000, CRC(089699f5) SHA1(2cc470a97936887804363c8783bad4db4cad4f64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mueve_reclam_ea76_pic_irn_27c020.u1", 0x00000, 0x40000, CRC(f3cc6936) SHA1(35334aeb85f3524f2afdf20f49005d7573ec5494) )
ROM_END

// Based on the song by the Beatles.
ROM_START( obladi )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x2000, CRC(089699f5) SHA1(2cc470a97936887804363c8783bad4db4cad4f64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "obladi_reclam_5a5c_pic_irn_27c020.u1", 0x00000, 0x40000, CRC(a156f749) SHA1(f2bcbe5857e8ea6d96c2abe3051a5d02308dc963) )
ROM_END

// Based on the song composed by Rafael Pérez Botija.
ROM_START( susanita )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "m.irn_pic16c56.u3", 0x0000, 0x2000, CRC(089699f5) SHA1(2cc470a97936887804363c8783bad4db4cad4f64) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "susanita.u1", 0x00000, 0x40000, CRC(766868cb) SHA1(eb42dc46b865bc448052d9d67c840e51c49ce49a) )
ROM_END

} // anonymous namespace

GAME( 1998, futbol,       0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Futbol (set 1)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1997, futbola, futbol, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Futbol (set 2)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 1997, futbolt, futbol, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Futbol (test ROMs)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

GAME( 199?, autopapa,     0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", u8"El auto de papá",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, donpepito,    0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Don Pepito",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, mueve,        0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Mueve",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, obladi,       0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Ob-La-Di",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 199?, susanita,     0, gaelcof3, gaelcof3, gaelcof3_state, empty_init, ROT0, "Gaelco / Cresmatic", "Susanita",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
