// license:BSD-3-Clause
// copyright-holders:AJR, Roberto Fresca
/*

VTech Genius Junior series

CPU is 68HC05 derived?

Other known undumped international versions:
- Genius PRO (French version of Genius Leader Select)
- Pitagorín Plus (Spanish version of Genius Junior Redstar 3)
- PreComputer Notebook (alternate English version of Genius Leader Notebook)
- Smart Start Animated (alternate English version of Genius Junior Redstar)
- El Super-Ordenador Parlanchín (alternate Spanish version of Genius Junior Movie)
- Talande Smart Start Laptop (Swedish version of Genius Junior Movie)
- Talking Whiz-Kid Animated (English version of Genius Junior Redstar)
- Talking Whiz-Kid Genius (English version of Genius Junior Movie)
- Talking Whiz-Kid Notebook (English version of Genius Leader Notebook)
- Talking Whiz-Kid Power Mouse (English version of Genius Leader Select)
- Talking Whiz-Kid Super Animated (English version of Genius Junior Redstar 3)

Undumped VTech laptops possibly on similar hardware:
- Genius Einstein (French version of Genius Leader Action)
- Genius Explorations (French version; German version unknown)
- Genius Junior Profi 2
- Genius Junior Profi 3
- Genius Leader Action
- Genius Leader Notebook Plus
- Genius Master Notebook MM
- Genius Master Power Maus
- Genius Prodige (French version of Genius Master Power Maus)
- Genius Progress (French version of Genius Junior Profi 2)
- Mega Ratón Parlanchín (Spanish version of Genius Explorations)
- PreComputer Notebook II (English version of Genius Leader Notebook Plus)
- Smart Start Future (alternate English version of Genius Junior Profi 2)
- Talking Einstein (English version of Genius Leader Action)
- Talking Whiz-Kid Explorer (alternate English version of Genius Leader Action)
- Talking Whiz-Kid Honors (alternate English version of Genius Junior Notebook Plus)
- Talking Whiz-Kid Laptop (English version of Genius Junior Profi 2)
- Talking Whiz-Kid Lessons (English version of Genius Master Power Maus)
- Talking Whiz-Kid Major Mouse (alternate English version of Genius Explorations)
- Talking Whiz-Kid Notebook 2000 (English version of Genius Master Notebook MM)
- Talking Whiz-Kid Notebook 3000 (alternate English version of Genius Master Notebook MM)
- Talking Whiz-Kid Power Mouse Deluxe (English version of Genius Explorations)

*/

/***************************************************************************************

    Product name:    Pitagorín Junior.
    Brand:           VTech.
    Type:            First steps (4-6 years old) laptop.
    Language:        Spanish.
    Description:     23 didactic games with voice and sounds for 1 or 2 players.
                     (simple maths operations, spell, hangman, letters, numbers, etc)

    Docs by Roberto Fresca.

  ***************************************************************************************

  Games / Activities ...

  ORDER  TITLE                       TRANSLATION
  -----+---------------------------+----------------------
   01  - La letra perdida.           The missing letter.
   02  - Deletrear.                  Spell.
   03  - Plurales.                   Plurals.
   04  - Verbos.                     Verbs.
   05  - El Ahorcado.                Hangman.
   06  - Revoltijo de letras.        Messed letters.
   07  - La palabra escondida.       The hidden word.
   08  - Trueque de letras.          Swapped letters.
   09  - La letra intrusa.           The intruder letter.
   10  - Matematicas (+,-,x,/).      Mathematics (+,-,x,/).
   11  - Aprendiendo los numeros.    Learning the numbers.
   12  - Redondeando cifras.         Rounding numbers.
   13  - Encuentra el signo.         Find the sign.
   14  - Calculadora.                Calculator.
   15  - Tres en raya.               Three in a row.
   16  - El juego de los puntos.     The dot's game.
   17  - El juego del squash.        The squash game.
   18  - El juego del arquero.       The archer game.
   19  - Dibujos animados.           Animated cartoons.
   20  - El compositor.              The composer.

  ***************************************************************************************

  What's inside....

  PCB silkscreened '9817'
      etched on copper '35-19122-2' & '703013-C'

  1x Unknown CPU inside an epoxy blob (more than 100 connections) @ U? (covered with the blob).
  1x VTech LH532HJT mask ROM (originary from Sharp) also silkscreened '9811D' @ U3.
  1x Texas Instruments TSP50C10 (CSM10150AN) speech synth with 8-bit microprocessor @ U2.
  1x SN74HC00N @ U5.
  1x SN74HC244N @ U4.

  1x Unknown oscillator (XTAL1).
  1x Unknown trimpot on an r/c oscillator (XTAL2).

  1x 32 contacts (single side) expansion port.
  1x 3 contacts (unknown) connector.
  1x 17 contacts Keyboard (KEY1) connector.
  1x 3 contacts (CONT) connector.


  PCB layout:
                         .......CONNECTORS........
  .------------------------------------------------------------------------------------.
  | .-----------------.  ooo ooooooooooooooooo ooo                         9817        |.---.
  | |   THIS SECTOR   |  unk        KEY1       CONT                      .-------.     /   =|
  | |                 |                                                  |       |    /    =|
  | |  IS POPULATED   |                                                  | VTECH |   | E   =|
  | |                 |            .-----------.                         |       |   | X   =|
  | |   WITH A LOT    |            | SN74HC00N |      (84C91)            | LH532 |   | P P =|
  | |                 |            '-----------'    .----------.         |  HJT  |   | A O =|
  | |      OF...      |                 U5          |CSM10150AN|         |       |   | N R =|
  | |                 |                             '----------'.---.    | 9811D |   | S T =|
  | |   RESISTORS,    |                                  U2     | / |    |       |   | I   =|
  | |                 |   U4                                    '---'    |       |   | O   =|
  | |   CAPACITORS,   |  .--.                                   XTAL 2   '-------'   | N   =|
  | |                 |  |SN|                          ____                 U3        \    =|
  | |      AND        |  |74|                         /    \                           \   =|
  | |                 |  |HC|         35-19122-2     | BLOB |                          |'---'
  | |  TRANSISTORS    |  |24|          703013-C       \____/              .----.       |
  | |                 |  |4N|                           U?                '----'       |
  | '-----------------'  '--'                                             XTAL 1       |
  '------------------------------------------------------------------------------------'


  Expansion Port:

  CONNECTOR                     CONNECTOR
  ---------                     ---------
  01 ----> Vcc                  17 ----> LH532HJT (pin 09)
  02 ----> Vcc                  18 ----> LH532HJT (pin 25)
  03 ----> GND                  19 ----> LH532HJT (pin 10)
  04 ----> ???                  20 ----> LH532HJT (pin 23)
  05 ----> LH532HJT (pin 03)    21 ----> LH532HJT (pin 11)
  06 ----> LH532HJT (pin 02)    22 ----> LH532HJT (pin 21)
  07 ----> LH532HJT (pin 04)    23 ----> LH532HJT (pin 12)
  08 ----> LH532HJT (pin 30)    24 ----> LH532HJT (pin 20)
  09 ----> LH532HJT (pin 05)    25 ----> LH532HJT (pin 13)
  10 ----> LH532HJT (pin 29)    26 ----> LH532HJT (pin 19)
  11 ----> LH532HJT (pin 06)    27 ----> LH532HJT (pin 14)
  12 ----> LH532HJT (pin 28)    28 ----> LH532HJT (pin 18)
  13 ----> LH532HJT (pin 07)    29 ----> LH532HJT (pin 15)
  14 ----> LH532HJT (pin 27)    30 ----> LH532HJT (pin 17)
  15 ----> LH532HJT (pin 08)    31 ----> GND
  16 ----> LH532HJT (pin 26)    32 ----> GND


  U3 - VTech LH532HJT (9811D) 2Mb mask ROM.
       Seems to be 27C020 pin compatible.

                .----v----.
          VCC --|01     32|-- VCC
              --|02     31|--
              --|03     30|--
              --|04     29|--
              --|05     28|--
              --|06     27|--
              --|07     26|--
              --|08     25|--
              --|09     24|--
              --|10     23|--
              --|11     22|--
              --|12     21|--
              --|13     20|--
              --|14     19|--
              --|15     18|--
          GND --|16     17|--
                '---------'


  U2 - Texas Instruments TSP50C10 (CSM10150AN).

       Speech Generator with 8-bit microprocessor, 8K ROM, 112 bytes RAM.
       Maximum Clock Frequency = 9.6 MHz.
       Package = DIP16
       Technology = CMOS

                 .---v---.
               --|01   16|--
               --|02   15|--
               --|03   14|--
               --|04   13|--
           GND --|05   12|-- VCC
               --|06   11|--
               --|07   10|-- GND
               --|08   09|--
                 '-------'

  ***************************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68hc05.h"
#include "softlist_dev.h"


namespace {

class geniusjr_state : public driver_device
{
public:
	geniusjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
	{
	}

	void gj4000(machine_config &config);
	void gln(machine_config &config);
	void gls(machine_config &config);
	void gj5000(machine_config &config);
	void gjrstar(machine_config &config);
	void gjmovie(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void gj4000_map(address_map &map) ATTR_COLD;
	void gj5000_map(address_map &map) ATTR_COLD;
	void gjrstar_map(address_map &map) ATTR_COLD;

	required_device<m68hc05_device> m_maincpu;
	required_memory_bank m_rombank;

	u16 m_bank_size;
};

void geniusjr_state::gj4000_map(address_map &map)
{
	map(0x8000, 0xffff).bankr("rombank");
}

void geniusjr_state::gj5000_map(address_map &map)
{
	map(0x4000, 0x7fff).bankr("rombank");
}

void geniusjr_state::gjrstar_map(address_map &map)
{
	map(0x2000, 0x3fff).bankr("rombank");
}


INPUT_PORTS_START( geniusjr )
INPUT_PORTS_END


void geniusjr_state::machine_start()
{
	memory_region *extrom = memregion("extrom");

	m_rombank->configure_entries(0, extrom->bytes() / m_bank_size, extrom->base(), m_bank_size);
	m_rombank->set_entry(0);
}

void geniusjr_state::gj4000(machine_config &config)
{
	M68HC05L9(config, m_maincpu, 8'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusjr_state::gj4000_map);

	m_bank_size = 0x8000;

	SOFTWARE_LIST(config, "cart_list").set_original("gj4000");
}

void geniusjr_state::gln(machine_config &config)
{
	gj4000(config);

	subdevice<software_list_device>("cart_list")->set_original("gln");
}

void geniusjr_state::gj5000(machine_config &config)
{
	M68HC05L9(config, m_maincpu, 8'000'000); // unknown clock (type also uncertain)
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusjr_state::gj5000_map);

	m_bank_size = 0x4000;

	SOFTWARE_LIST(config, "cart_list").set_original("gj4000");
}

void geniusjr_state::gjrstar(machine_config &config)
{
	M68HC05L9(config, m_maincpu, 8'000'000); // unknown clock (type also uncertain, could be L7 instead of L9)
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusjr_state::gjrstar_map);

	m_bank_size = 0x2000;

	SOFTWARE_LIST(config, "cart_list").set_original("gjrstar");
}

void geniusjr_state::gjmovie(machine_config &config)
{
	gjrstar(config);

	subdevice<software_list_device>("cart_list")->set_original("gjmovie");
}

void geniusjr_state::gls(machine_config &config)
{
	gjrstar(config);

	subdevice<software_list_device>("cart_list")->set_original("gls");
}


ROM_START( gj4000 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-05886-000-000.u4", 0x000000, 0x40000, CRC(5f6db95b) SHA1(fe683154e33a82ea38696096616d11e850e0c7a3))
ROM_END

// VTech PCB 35-21205. "C.Q.F.D" is a VTech brand, and the Scientus is a straight clone of the "Genius Junior 4000".
ROM_START( scientus )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x80000, "extrom", 0 )
	ROM_LOAD( "54-6050-00-0.u2",  0x000000, 0x80000, CRC(dbcfebaa) SHA1(863697d144857fab45aad493b812ed607ad7e1d0)) // AMD AM27C010

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "27-05992-0-0.u3", 0x0000, 0x2000, NO_DUMP ) // TI speech chip
ROM_END

ROM_START( gj5000 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x80000, "extrom", 0 )
	ROM_LOAD( "27-6019-01.u2", 0x000000, 0x80000, CRC(946e5b7d) SHA1(80963d6ad80d49e54c8996bfc77ac135c4935be5))
ROM_END

ROM_START( gjmovie )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "lh532hlk.bin", 0x000000, 0x40000, CRC(2e64c296) SHA1(604034f902e20851cb9af60964031a508ceef83e))
ROM_END

ROM_START( pitagjr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "lh532hjt_9811d.u3", 0x00000, 0x40000, CRC(23878b45) SHA1(8f3c41c10cfde9d76763c3a8701ec6616db4ab40) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "csm10150an.u2", 0x0000, 0x2000, NO_DUMP ) // TSP50C10 (8K bytes of ROM) labeled "CSM10150AN"
ROM_END

ROM_START( gjrstar )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-5740-00.u1", 0x000000, 0x40000, CRC(ff3dc3bb) SHA1(bc16dfc1e12b0008456c700c431c8df6263b671f))
ROM_END

ROM_START( gjrstar2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-5740-00.u1", 0x000000, 0x40000, CRC(ff3dc3bb) SHA1(bc16dfc1e12b0008456c700c431c8df6263b671f)) // Identical to 'Genius Junior Redstar'
ROM_END

// VTech PCB 35-10100-01
ROM_START( pcompelr )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.u8", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "speech.u7", 0x0000, 0x2000, NO_DUMP ) // Labeled "930 0AF21FK / VTECH / ©YY 04044"

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-05944-000-001.u1", 0x000000, 0x40000, CRC(5018763d) SHA1(70e1d8b8e34e0b2ab10d7ac06c2f454d1f377e77)) // Dumped as AM27C020, pin 1 connected to 32 (Vcc), as per 27c020 specs)
ROM_END

ROM_START( gjrstar3 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "54-06056-000-000.u3", 0x000000, 0x040000, CRC(72522179) SHA1(ede9491713ad018012cf925a519bcafe126f1ad3))
ROM_END

ROM_START( gln )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x80000, "extrom", 0 )
	ROM_LOAD( "27-5308-00_9524_d.bin", 0x000000, 0x080000, CRC(d1b994ee) SHA1(b5cf0810df0676712e4f30e279cc46c19b4277dd))
ROM_END

ROM_START( pitagor )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x80000, "extrom", 0 )
	ROM_LOAD( "27-5374-00.u2", 0x000000, 0x80000, CRC(89a8fe7d) SHA1(dff06f7313af22c6c19b1f00c0651a64cc505fe2))

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "csm10150an.u1", 0x0000, 0x2000, NO_DUMP ) // TSP50C10 (8K bytes of ROM) labeled "64C_4TT VIDEO TECH CSM10150AN"
ROM_END

ROM_START( gls )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x2000, NO_DUMP ) // As per decap, confirmed to be a Motorola 68HC05 CSIC (Customer Specification Integrated Circuit)

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-5635-00.u2", 0x000000, 0x40000, CRC(bc3c0587) SHA1(fe98f162bd80d96ce3264087b5869f4505955464))
ROM_END

} // anonymous namespace


//    YEAR   NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY    FULLNAME                             FLAGS
COMP( 1996,  gj4000,   0,       0,      gj4000,   geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Junior 4000 (Germany)",      MACHINE_IS_SKELETON )
COMP( 1999?, scientus, gj4000,  0,      gj4000,   geniusjr, geniusjr_state, empty_init, "C.Q.F.D", "Scientus (France)",                 MACHINE_IS_SKELETON )
COMP( 1993,  gjmovie,  0,       0,      gjmovie,  geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Junior Movie (Germany)",     MACHINE_IS_SKELETON )
COMP( 199?,  pitagjr,  gjmovie, 0,      gjmovie,  geniusjr, geniusjr_state, empty_init, "VTech",   "Pitagorin Junior",                  MACHINE_IS_SKELETON )
COMP( 1996,  gjrstar,  0,       0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Junior Redstar (Germany)",   MACHINE_IS_SKELETON )
COMP( 1996,  gjrstar2, gjrstar, 0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Junior Redstar 2 (Germany)", MACHINE_IS_SKELETON )
COMP( 1995,  pcompelr, gjrstar, 0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",   "Precomputer Elektronik (Russia)",   MACHINE_IS_SKELETON ) // Прекомпьютер Электроник
COMP( 1998,  gjrstar3, 0,       0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Junior Redstar 3 (Germany)", MACHINE_IS_SKELETON )
COMP( 1998,  gj5000,   0,       0,      gj5000,   geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Junior 5000 (Germany)",      MACHINE_IS_SKELETON )
COMP( 1993,  gln,      0,       0,      gln,      geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Leader Notebook",            MACHINE_IS_SKELETON )
COMP( 1993,  pitagor,  gln,     0,      gln,      geniusjr, geniusjr_state, empty_init, "VTech",   "Pitagorin",                         MACHINE_IS_SKELETON )
COMP( 1995,  gls,      0,       0,      gls,      geniusjr, geniusjr_state, empty_init, "VTech",   "Genius Leader Select",              MACHINE_IS_SKELETON )
