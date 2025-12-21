// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/***************************************************************************

  MANN, OH-MANN
  199? - MERKUR

  Preliminary driver by Roberto Fresca.


  German board game similar to Ludo, derived from the Indian game Parchisi.
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

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/mc68681.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/saa1099.h"
#include "speaker.h"


namespace {

class manohman_state : public driver_device
{
public:
	manohman_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_pit(*this, "pit")
	{ }

	void manohman(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;
	required_device<pit68230_device> m_pit;
};


void manohman_state::machine_start()
{
}


/*********************************************
*           Memory Map Definition            *
*********************************************/

void manohman_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x100000, 0x10003f).rw(m_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);
	map(0x200000, 0x20001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x300000, 0x300003).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff).nopr();
	map(0x400000, 0x40001f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
	map(0x500000, 0x503fff).ram().share("nvram"); //work RAM
	map(0x600002, 0x600003).nopw(); // output through shift register?
	map(0x600004, 0x600005).nopr();
	map(0x600006, 0x600007).noprw(); //(r) is discarded (watchdog?)
}

void manohman_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff4, 0xfffff5).r(m_pit, FUNC(pit68230_device::irq_tiack));
	map(0xfffff8, 0xfffff9).r(m_duart, FUNC(mc68681_device::get_irq_vector));
}

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

void manohman_state::manohman(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(8'000'000)); // MC68000P8
	m_maincpu->set_addrmap(AS_PROGRAM, &manohman_state::mem_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &manohman_state::cpu_space_map);

	PIT68230(config, m_pit, XTAL(8'000'000)); // MC68230P8
	m_pit->timer_irq_callback().set_inputline("maincpu", M68K_IRQ_2);

	MC68681(config, m_duart, XTAL(3'686'400));
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MSM6242(config, "rtc", XTAL(32'768)); // M62X42B

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE); // KM6264BL-10 x2 + MAX696CFL + battery

	SPEAKER(config, "mono").front_center();
	SAA1099(config, "saa", XTAL(8'000'000) / 2).add_route(ALL_OUTPUTS, "mono", 0.10); // clock not verified
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( manohman )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mom_austria_vorserie_ii.bin", 0x00000, 0x10000, CRC(4b57409c) SHA1(0438f5d52f4de2ece8fb684cf2d82bdea0eacf0b) )
	ROM_LOAD16_BYTE( "mom_austria_vorserie_i.bin",  0x00001, 0x10000, CRC(3c9507f9) SHA1(489a6aadfb7d61be0873bf48d428e9d915268f95) )
ROM_END

ROM_START( backgamn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b_f2_i.bin",  0x00000, 0x10000, CRC(9e42937c) SHA1(85d462a560b85b03ee9d341e18815b7c396118ac) )
	ROM_LOAD16_BYTE( "b_f2_ii.bin", 0x00001, 0x10000, CRC(8e0ee50c) SHA1(2a05c337db1131b873646aa4109593636ebaa356) )
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT   COMPANY   FULLNAME         FLAGS
GAME( 199?, manohman, 0,      manohman, manohman, manohman_state, empty_init, ROT0, "Merkur", "Mann, oh-Mann", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
GAME( 1990, backgamn, 0,      manohman, manohman, manohman_state, empty_init, ROT0, "Merkur", "Backgammon",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_REQUIRES_ARTWORK )
