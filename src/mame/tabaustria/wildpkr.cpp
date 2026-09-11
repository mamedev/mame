// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  Wild Poker
  TAB Austria.

  Preliminary driver by Roberto Fresca.


  Games running in this hardware:

  * Wild Poker (ver. D 1.01),         199?, TAB Austria.


  The HD63484 ACRTC support is incomplete,
  due to the preliminary emulation state.

*******************************************************************************

  Hardware Notes:
  ---------------

  CPU:
  - 1x MC68000P12        ; 68000 CPU @ 12 MHz, from Motorola.
  - 1x D8751H            ; 8751 MCU (3.6864 MHz?)

  Sound device:
  - 1x AY8930            ; Sound IC, from Microchip Technology.

  Video:
  - 1x HD63484CP8 @ 8MHz ; Advanced CRT Controller (ACRTC), from Hitachi Semiconductor.
  - 1x HD63485CP64       ; Hitachi - Graphic Memory Interface Controller (GMIC).
  - 2x HD63486CP32       ; Hitachi - Graphic Video Attribute Controller (GVAC).

  Other:
  - 1x MC68681           ; Motorola - Dual Asynchronous Receiver/Transmitter.
  - 4x XTALs....         ; 3.6864 / 12.000 / 26.000 / 24.000 MHz.

                                                                                          .--------.
  PCB Layout:                                                                           --+--------+--
  .---------------------------------------------------------------------------------------+        +-----------------------------------------------------.
  |                                                                                       |  DB9   |                                                     |
  |                        .--------.                    .--------.                       |        |                             .--------.              |
  |                        |::::::::|                    |::::::::|                       '--------'                             |74HCT32P|              |
  |                        '--------'                    '--------'                                                              '--------'              |
  |                        .--------.                    .---------. .----------.         .--------. .----------.  .----------.  .--------.              |
  |                        |LT1084CN|                    |SN75116N | | MM57410N |         |74HCT14P| |74HCT245P |  |74HCT245P |  |74HCT86P| .-------.    |
  |                        '--------'                    '---------' '----------'         '--------' '----------'  '----------'  '--------' |XTAL 3 |    |
  |                                                                                                                  .--------.   .-------. |       |    |
  |                                         .--------.    .---------.        .-..-.                                  |        |   |DM74S04| '-------'    |
  |                                         |  PC617 |    |74HCT14P |        | || |               .----------------. '--------'   '-------'              |
  |                                         '--------'    '---------'        '-''-'               |  inmos 8941-C  |                                     |
  |.--. .---------.  .---------.    .-----------. .--------.     .---------------.                |  IMS G176P-50  |           .----------. .----------. |
  ||..| |ULN2803A |  |74HCT533P|    |PC74HC245P | |74HCT125|     |    HYUNDAI    |                |                |           |HY53C464LS| |HY53C464LS| |
  ||..| '---------'  '---------'    '-----------' '--------'     |  HY6264LP-10  |                '----------------'           '----------' '----------' |
  ||..|                           .------------------------.     |  9040D KOREA  |  .-------------.        .-------------.     .----------. .----------. |
  ||..| .---------.  .---------.  |        AY8930 /P       |     '---------------'  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  ||..| |ULN2803A |  |74HCT533P|  |        9019CCA         |                        |             |        |             |     '----------' '----------' |
  ||..| '---------'  '---------'  |        TAIWAN          |     .---------------.  |    IE1 U    |        |    9117     |     .----------. .----------. |
  ||..|                           '------------------------'     |    HYUNDAI    |  | HD63484CP8  |        | HD63486CP32 |     |HY53C464LS| |HY53C464LS| |
  ||..| .---------.  .---------.                                 |  HY6264LP-10  |  |             |        |             |     '----------' '----------' |
  ||..| |ULN2803A |  |74HCT533P|       .--------. .--------.     |  9040D KOREA  |  |        Japan|        |        Japan|     .----------. .----------. |
  ||..| '---------'  '---------'  .---.|8      1| |8      1|     '---------------'  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  |'--'                           |   ||  DSW1  | |  DSW2  |                        |             |        |             |     '----------' '----------' |
  |     .---------.  .---------.  '---''--------' '--------'                        '-------------'        '-------------'                               |
  |.--. |ULN2803A |  |74HCT533P|                                                                                                                         |
  ||..| '---------'  '---------'  .------------------------.                                                                   .----------. .----------. |
  ||..|                           |D8751H                  |                                                                   |HY53C464LS| |HY53C464LS| |
  ||..|                           |L0381103                |                        .-------------.        .-------------.     '----------' '----------' |
  ||..|  .--------.  .---------.  |          VD1.00        |  .------------------.  |             |        |             |     .----------. .----------. |
  ||..|  |MDP1603 |  |74HCT245P|  '------------------------'  |D27C020           |  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  |'--'  '--------'  '---------'  .-------.     .-------.     |                  |  |     9109    |        |    9117     |     '----------' '----------' |
  |                               |XTAL 1 |     |XTAL 2 |     |   VD / 1.01 / 3  |  | HD63485CP64 |        | HD63486CP32 |     .----------. .----------. |
  |.--.  .--------.  .---------.  |       |     |       |     '------------------'  |             |        |             |     |HY53C464LS| |HY53C464LS| |
  ||..|  |MDP1603 |  |74HCT245P|  '-------'     '-------'                           |        Japan|        |        Japan|     '----------' '----------' |
  ||..|  '--------'  '---------'  .------------------------.                        |             |        |             |     .----------. .----------. |
  ||..|                           |        MC68681P        |                        |             |        |             |     |HY53C464LS| |HY53C464LS| |
  ||..|  .--------.  .---------.  |         2C98R          |                        '-------------'        '-------------'     '----------' '----------' |
  ||..|  |MDP1603 |  |74HCT245P|  |        QQPQ9051        |                                                                                             |
  ||..|  '--------'  '---------'  '------------------------'  .------------------.                                             .--------.   .--------.   |
  ||..|                             .--------.    .--------.  |D27C020           |                                             |        |   |        |   |
  ||..|  .--------.  .---------.    |8      1|    |74HCT147|  |                  |                                             '--------'   '--------'   |
  ||..|  |MDP1603 |  |74HCT245P|    |  DSW3  |    '--------'  |   VD / 1.01 / 1  |                 .--------.  .--------.                                |
  ||..|  '--------'  '---------'    '--------'                '------------------'                 |74HCT138|  |74HCT74P|                                |
  |'--'                                                                     .----------. .-------. '--------'  '--------'                           .------.
  |                             .---------------------------------------.   | GAL16V8S | |74HCT74|                                                  |      |
  |      .-------.              |                                       |   '----------' '-------' .--------.  .--------.                           |      |
  |      |       |              |             MC68000P12                |  .--------.   .------.   |74HCT138|  |74HCT21P|                           |      |
  |      |Battery|              |               2C91E                   |  |74HCT04P|   |XTAL 4|   '--------'  '--------'                           |      |
  |      |       |              |              QZUZ9102                 |  '--------'   |      |   .--------.  .--------.                           |      |
  |      |       |              |                                       |  .--------.   '------'   |74HCT138|  |74HCT161|                           |      |
  |      '-------'              '---------------------------------------'  |74HCT14P|              '--------'  '--------'                           |      |
  |                                                                        '--------'  .-------.   .--------.  .--------.                           |      |
  |               .--.  .--.                                                           |74HCT08|   |74HCT21 |  |1      8|                           |      |
  |               |TL|  |TL|                                                           '-------'   '--------'  |  DSW4  |                           |      |
  |               '--'  '--'    ========================================                                       '--------'                           '------'
  |                            | |::::::::::::::::::::::::::::::::::::| |                                                                                |
  |                            | |::::::::::::::::::::::::::::::::::::| |                                                                                |
  |                             ========================================                                                                                 |
  '------------------------------------------------------------------------------------------------------------------------------------------------------'

  XTAL 1: 3.6864 MHz.
  XTAL 2: 12.000 MHz.
  XTAL 3: 26.000 MHz.
  XTAL 4: 24.000 MHz.

  TL: TL7705ACP


      DSW1:          DSW2:          DSW3:          DSW4:
   .--------.     .--------.     .--------.     .--------.
  1| oo oooo|8   1|oooooooo|8   1|oooooooo|8   1|  o     |8    ON
   |--------|     |--------|     |--------|     |--------|
   |o  o    |     |        |     |        |     |oo ooooo|     OFF
   '--------'     '--------'     '--------'     '--------'


*******************************************************************************

  *** Game Notes ***

  Nothing yet...


*******************************************************************************

  ---------------------------------
  ***  Memory Map (preliminary) ***
  ---------------------------------

  00000 - 7FFFF  ; ROM space.


*******************************************************************************

  DRIVER UPDATES:

  [2012-06-11]

  - Initial release.
  - Pre-defined Xtals.
  - Added ASCII PCB layout.
  - Started a preliminary memory map.
  - Added technical notes.


  TODO:

  - Improve memory map.
  - ACRTC support.
  - GFX decode.
  - Sound support.
  - A lot!.


*******************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/hd63484.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class wildpkr_state : public driver_device
{
public:
	wildpkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart")
	{ }

	void wildpkr(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart;

	u8 unknown_read8();
	void unknown_write8(u8 data);

	void hd63484_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};


/*************************
*     Video Hardware     *
*************************/

void wildpkr_state::video_start()
{
}

/*************************
*      ACRTC Access      *
*************************/


/*************************
*      Misc Handlers     *
*************************/

u8 wildpkr_state::unknown_read8()
{
	return 0xff;
}

void wildpkr_state::unknown_write8(u8 data)
{
}


/*************************
*      Memory Map        *
*************************/

void wildpkr_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x113fff).ram();
	map(0x800000, 0x800003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x800080, 0x80009f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x800180, 0x800180).r(FUNC(wildpkr_state::unknown_read8));
	map(0x800181, 0x800181).w(FUNC(wildpkr_state::unknown_write8));
	map(0x800200, 0x800200).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x800202, 0x800202).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x800204, 0x800204).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x800280, 0x800280).w("aysnd", FUNC(ay8930_device::data_w));
	map(0x800282, 0x800282).w("aysnd", FUNC(ay8930_device::address_w));
	map(0x800285, 0x800285).r("aysnd", FUNC(ay8930_device::data_r)); // (odd!)
	map(0x800286, 0x800289).nopw();
}

void wildpkr_state::hd63484_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}

/* Unknown R/W:


*/


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( wildpkr )
INPUT_PORTS_END


/*************************
*     Machine Start      *
*************************/

void wildpkr_state::machine_start()
{
/*
  ACRTC memory:

  00000-3ffff = RAM
  40000-7ffff = ROM
  80000-bffff = unused
  c0000-fffff = unused
*/
}


void wildpkr_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


/*************************
*    Machine Drivers     *
*************************/

void wildpkr_state::wildpkr(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wildpkr_state::program_map);
	//m_maincpu->set_vblank_int("screen", FUNC(wildpkr_state::irq2_line_hold)); // guess

	MC68681(config, m_duart, 3.6864_MHz_XTAL);

	// AUX1_CLOCK  XTAL(26'000'000)
	// AUX2_CLOCK  XTAL(24'000'000)

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(384, 280);
	screen.set_visarea(0, 384-1, 0, 280-1);
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette("palette");

	HD63484(config, "acrtc").set_addrmap(0, &wildpkr_state::hd63484_map);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", "palette"));
	ramdac.set_addrmap(0, &wildpkr_state::ramdac_map);

	PALETTE(config, "palette").set_entries(256);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8930(config, "aysnd", 12_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( wildpkr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vd_1.01_3.bin", 0x000000, 0x40000, CRC(d19d5609) SHA1(87eedb7daaa8ac33c0a73e4e849b9a0f76152261) )
	ROM_LOAD16_BYTE( "vd_1.01_1.bin", 0x000001, 0x40000, CRC(f10644ab) SHA1(5872fe41b8c7fec5e83011abdf82a85f064b734f) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "d8751h", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal6v8s.bin", 0x000, 0x117, CRC(389c63a7) SHA1(4ebb26a001ed14a9e96dd268ed1c7f298f0c086b) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT    STATE           INIT         ROT   COMPANY        FULLNAME                    FLAGS
GAME( 199?, wildpkr,   0,        wildpkr,  wildpkr, wildpkr_state,  empty_init,  ROT0, "TAB Austria", "Wild Poker (ver. D 1.01)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
