// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/***************************************************************************

  MANN, OH-MANN
  199? - MERKUR

  Preliminary driver by Roberto Fresca.


  German board game similar to Ludo, derivated from the indian game Parchisi.
  Coin-operated machine for 1-4 players. No screen, just artwork and lamps.
  The machine was designed for pubs, etc...

  Field: 93 cm.
  High:  105 cm.

  1x keylock.
  Accept DM and Euro.


  It's all a challenge. Even once emulated, the game will need a lot of
  artwork and lamps work...

  Currently sits in a loop between 0x100000 and 0x600006 addresses r/w,
  the snippet is there:

  001BB8: move.b  (A2), D0
  001BBA: jsr     $6dc0.l
  001BC0: tst.b   D0
  001BC2: bne     $1bb8

  Passing this loop it checks the i/o stuff, including the sound addresses

****************************************************************************

  Hardware Notes...

  - XTAL1 = 8.000 MHz.
  - XTAL2 = 3.6864 MHz.

  1x MC68000P8        ; Motorola, 16-bits CPU.
  1x SAA1099P         ; Philips, 6-Voice Sound Generator.
  2x LC3664BL-10      ; Sanyo, 64K Static RAM.
  1x M62X42B          : OKI, Real Time Clock with built in crystal.
  1x MC68230P8        ; Motorola, Parallel Interface / Timer.
  1x SCN68681C1N40    ; Philips, Dual Asynchronous Receiver/Transmitter (DUART).
  1x MAX696CP         ; Maxim, Microprocessor Supervisory Circuits.


  PCB Layout:
  .------------------------------------------------------.
  | .-------------.    .-----.    .---------.            |
  | |:::::::::::::|    |:::::|    |:::::::::|            |
  | '-------------'    '-----'    '---------'            |
  |         .------------------------.  .-------.        |
  | .-.     |       MC68230P8        |  | L4962 |        |
  | |.|     |         1C10R          |  '-------'        |
  | |.|     |         WC9336         |                   |
  |R|.|     '------------------------'                   |
  |E|.|                                                  |
  |S|.|                                                  |
  |E|.| .---------.      .-----------.                   |
  |R|.| |74HC245N |      | POWER     |                   |
  |V|.| '---------'      |  MODULE   |                   |
  |E|.|                  |           |                   |
  | |.| .---------.      |  3 VOLTS  |    .--------.     |
  | |.| |74HC273B1|      |           |    |MAX696CP|     |
  | '-' '---------'      |           |    '--------'     |
  |                      |           |                   |
  |     .--------.       '-----------'                   |
  |     |74HC4094|                                       |
  |     '--------' .-------------.   .-------------.     |
  |                |    SANYO    |   |    SANYO    |     |
  |  .--------.    | LC3664BL-10 |   | LC3664BL-10 |     |
  |  |74HC04B1|    |             |   |             |     |
  |  '--------'    '-------------'   '-------------'     |
  |  .--------.                                          |
  |  |74HC164B|                                          |
  |  '--------'    .-------------.   .-------------.     |
  |                |Mann,oh-Mann |   |Mann,oh-Mann |     |
  |                |Austria      |   |Austria      |     |
  |   .---. XTAL1  |Vorserie II  |   |Vorserie I   |     |
  |                '-------------'   '-------------'     |
  |  .--------.      .---------.       .---------.       |
  |  |74HC04B1|      |74HC245N |       |74HC245N |       |
  |  '--------'      '---------'       '---------'       |
  |                   .........         .........        |
  |                    8x10K             8x10K           |
  |  .--------.                                          |
  |  |74HC139N|    .--------------------------------.    |
  |  '--------'    |                                |    |
  |                |           MC68000P8            |    |
  |                |                                |    |
  |  .--------.    |                                |    |
  |  |74HC30B1|    '--------------------------------'    |
  |  '--------'          .........  .........            |
  |                        8x10K      8x10K              |
  |  .--------.        .--------.  .----------.          |
  |  |74HC32N |        |74HC138B|  | 74HC245N |          |
  |  '--------'        '--------'  '----------'          |
  |  .--------.       .---------.                        |
  |  |74HC00B1|       | M62X42B |                        |
  |  '--------'       '---------'                        |
  |  .--------.       .---------.                        |
  |  |74HC74B1|       |SAA1099P |                        |
  |  '--------'       '---------'                        |
  |           .---. XTAL2                                |
  | . .------------------------.                         |
  |8. |                        |                         |
  |x. |     SCN68681C1N40      |                         |
  |1. |                        |                         |
  |0. '------------------------'                         |
  |K.     .........                                      |
  | .       8x10K                                        |
  |  .--------.  .--------.  .--------.  .----.  .--.    |
  |  |::::::::|  |::::::::|  |::::::::|  |::::|  |::|    |
  |  '--------'  '--------'  '--------'  '----'  '--'    |
  |   SERVICE                 SERIAL1   SERIAL2 SPEAKER  |
  '------------------------------------------------------'


****************************************************************************

  Memory Map:
  -----------

  000000-01FFFF   ROM Space.
  500000-503FFF   RAM.


***************************************************************************/

#define MASTER_CLOCK        XTAL_8MHz
#define SECONDARY_CLOCK     XTAL_3_6864MHz

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/saa1099.h"


class _manohman_state : public driver_device
{
public:
	_manohman_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;
};


/*********************************************
*           Memory Map Definition            *
*********************************************/

static ADDRESS_MAP_START( manohman_map, AS_PROGRAM, 16, _manohman_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP     // smell to MAX696 watchdog...
	AM_RANGE(0x300000, 0x300001) AM_DEVWRITE8("saa", saa1099_device, data_w, 0x00ff)
	AM_RANGE(0x300002, 0x300003) AM_DEVWRITE8("saa", saa1099_device, control_w, 0x00ff)
	AM_RANGE(0x500000, 0x503fff) AM_RAM
	AM_RANGE(0x600006, 0x600007) AM_RAM     // write bitpatterns to compare with the 500000-503ff8 RAM testing.
//  AM_RANGE(0xYYYYYY, 0xYYYYYY) AM_RAM
ADDRESS_MAP_END

/*

  RW

  100000 ; R      \
  100000 ; W 0000  | Constant after RAM test... Seems for the MAX696's watchdog.
  100000 ; W 00FF /

  500000-503FF9 ; R
  500000-503FF9 ; W FFFF \
  500000-503FF9 ; W AAAA  | Seems bit patterns for testing RAM...
  500000-503FF9 ; W 5555  |
  500000-503FF9 ; W 0000 /

  503FFA - 503FFF RW

  500300 ; R
  500302 ; R

  600006 ; R
  600006 ; W FFFF \
  600006 ; W AAAA  | These bit patterns are for 500000-503ff8 comparison.
  600006 ; W 5555  |
  600006 ; W 0000 /


  BP at 0x1880 to point to the end of RAM test.

*/

/*********************************************
*          Input Ports Definitions           *
*********************************************/

static INPUT_PORTS_START( manohman )

INPUT_PORTS_END



/*********************************************
*               Machine Config               *
*********************************************/

static MACHINE_CONFIG_START( manohman, _manohman_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK)   // 8 MHz
	MCFG_CPU_PROGRAM_MAP(manohman_map)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SAA1099_ADD("saa", MASTER_CLOCK /* guess */)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( manohman )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "mom_austria_vorserie_ii.bin", 0x000000, 0x010000, CRC(4b57409c) SHA1(0438f5d52f4de2ece8fb684cf2d82bdea0eacf0b) )
	ROM_LOAD16_BYTE( "mom_austria_vorserie_i.bin",  0x000001, 0x010000, CRC(3c9507f9) SHA1(489a6aadfb7d61be0873bf48d428e9d915268f95) )
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT    ROT    COMPANY   FULLNAME        FLAGS... */
GAME( 199?, manohman, 0,      manohman, manohman, driver_device, 0,      ROT0, "Merkur", "Mann, oh-Mann", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
