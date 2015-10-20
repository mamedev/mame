// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/***************************************************************************************

    Product name:    Pitagorin Junior.
    Brand:           VTech.
    Type:            First steps (4-6 years old) laptop.
    Language:        Spanish.
    Description:     23 didactic games with voice and sounds for 1 or 2 players.
                     (simple maths operations, spell, hang man, letters, numbers, etc)

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
   15  - Tres en raya.               Three in a raw.
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
  1x Texas Instruments 84C91HT (CSM10150AN) speech synth with 8-bit microprocessor @ U2.
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


  U2 - Texas Instruments 84C91HT (CSM10150AN).

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
#include "cpu/m6805/m6805.h"
#include "rendlay.h"

class pitagjr_state : public driver_device
{
public:
	pitagjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_rombank(*this, "rombank")
		{ }

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;

	virtual void machine_start();
	DECLARE_PALETTE_INIT(pitagjr);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


static ADDRESS_MAP_START(pitajr_mem, AS_PROGRAM, 8, pitagjr_state)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_ROM // boot ROM ???
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("rombank")
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( pitajr )
INPUT_PORTS_END

void pitagjr_state::machine_start()
{
	m_rombank->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x2000);
	m_rombank->set_entry(1);
}

PALETTE_INIT_MEMBER(pitagjr_state, pitagjr)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

UINT32 pitagjr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static MACHINE_CONFIG_START( pitajr, pitagjr_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD63705, XTAL_2MHz)   // probably a m6805-based MCU with internal boot ROM
	MCFG_CPU_PROGRAM_MAP(pitajr_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(pitagjr_state, screen_update)
	MCFG_SCREEN_SIZE( 200, 100 )    // FIXME
	MCFG_SCREEN_VISIBLE_AREA( 0, 200-1, 0, 100-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(pitagjr_state, pitagjr)
MACHINE_CONFIG_END

/* ROM definition */

ROM_START( pitagjr )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "lh532hjt_9811d.u3", 0x00000, 0x40000, CRC(23878b45) SHA1(8f3c41c10cfde9d76763c3a8701ec6616db4ab40) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 199?, pitagjr,    0,       0,  pitajr,   pitajr, driver_device,     0,  "VTech",   "Pitagorin Junior", MACHINE_IS_SKELETON)
