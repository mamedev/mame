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
  - 1x AY8930            ; Sound IC, from Yamaha.

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


#define MAIN_CLOCK  XTAL_12MHz
#define SEC_CLOCK   XTAL_3.6864MHz
#define AUX1_CLOCK  XTAL_26MHz
#define AUX2_CLOCK  XTAL_24MHz

#include "emu.h"
#include "cpu/m68000/m68000.h"
//#include "video/hd63484.h"
#include "video/ramdac.h"


class wildpkr_state : public driver_device
{
public:
	wildpkr_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	DECLARE_DRIVER_INIT(wildpkr);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(wildpkr);
	UINT32 screen_update_wildpkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


/*************************
*     Video Hardware     *
*************************/

void wildpkr_state::video_start()
{
}

UINT32 wildpkr_state::screen_update_wildpkr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

PALETTE_INIT_MEMBER(wildpkr_state, wildpkr)
{
}


/*************************
*      ACRTC Access      *
*************************/


/*************************
*      Misc Handlers     *
*************************/


/*************************
*      Memory Map        *
*************************/

static ADDRESS_MAP_START( wildpkr_map, AS_PROGRAM, 16, wildpkr_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
//  AM_RANGE(0x800000, 0x800003) ACRTC?
	AM_RANGE(0x800180, 0x800181) AM_READNOP // protection, puts m68k code snippets to RAM
	AM_RANGE(0x800200, 0x800201) AM_DEVWRITE8("ramdac", ramdac_device, index_w, 0xff00)
	AM_RANGE(0x800202, 0x800203) AM_DEVWRITE8("ramdac", ramdac_device, pal_w, 0xff00)
	AM_RANGE(0x800204, 0x800205) AM_DEVWRITE8("ramdac", ramdac_device, mask_w, 0xff00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tabpkr_map, AS_PROGRAM, 16, wildpkr_state )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
	AM_RANGE(0x300000, 0x303fff) AM_RAM
	AM_RANGE(0x400000, 0x4007ff) AM_RAM // dallas timekeeper?

//	AM_RANGE(0x800200, 0x800201) AM_DEVWRITE8("ramdac", ramdac_device, index_w, 0xff00)
//	AM_RANGE(0x800202, 0x800203) AM_DEVWRITE8("ramdac", ramdac_device, pal_w, 0xff00)
//	AM_RANGE(0x800204, 0x800205) AM_DEVWRITE8("ramdac", ramdac_device, mask_w, 0xff00)
ADDRESS_MAP_END

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


static ADDRESS_MAP_START( ramdac_map, AS_0, 8, wildpkr_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb666_w)
ADDRESS_MAP_END



/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( wildpkr, wildpkr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(wildpkr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wildpkr_state,  irq1_line_hold)   //guess


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(wildpkr_state, screen_update_wildpkr)
	MCFG_SCREEN_PALETTE("palette")

//  MCFG_DEVICE_ADD("hd63484", HD63484, 0)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(wildpkr_state, wildpkr)

MACHINE_CONFIG_END


static MACHINE_CONFIG_START( tabpkr, wildpkr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)
	MCFG_CPU_PROGRAM_MAP(tabpkr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", wildpkr_state,  irq2_line_hold)   // 2 / 5 are valid


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(wildpkr_state, screen_update_wildpkr)
	MCFG_SCREEN_PALETTE("palette")

//  MCFG_DEVICE_ADD("hd63484", HD63484, 0)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(wildpkr_state, wildpkr)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( wildpkr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vd_1.01_3.bin", 0x000000, 0x40000, CRC(d19d5609) SHA1(87eedb7daaa8ac33c0a73e4e849b9a0f76152261) )
	ROM_LOAD16_BYTE( "vd_1.01_1.bin", 0x000001, 0x40000, CRC(f10644ab) SHA1(5872fe41b8c7fec5e83011abdf82a85f064b734f) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "d8751h",  0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal6v8s.bin",  0x0000, 0x0117, CRC(389c63a7) SHA1(4ebb26a001ed14a9e96dd268ed1c7f298f0c086b) )
ROM_END

/* seems to be different hardware, but same basic video chips, keep here or move?

cpu 68000-16
Xtal 24Mhaz
cpu ram 2x 6264

Audio DAC AD557JN

video area
insg176p-66 ramdac?
hd63487cp  Memory interface and video attribute controller
hd63484cp8 advanced CRT controller
4x km44c258cz-10 rams

*/

ROM_START( tabpkr )
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop1851.bin", 0x000001, 0x80000, CRC(fbe13fa8) SHA1(7c19b6b4d9a9935b6feb70b6261bafc6d9afb59f) )
	ROM_LOAD16_BYTE( "rop1853.bin", 0x000000, 0x80000, CRC(e0c312b4) SHA1(57c64c82f723067b7b2f9bf3fdaf5aedeb4f9dc3) )
	// are these missing, or just unpopulated but checked anyway?
	/* reads 0x100000 - 0x1fffff ? - 2x sockets for same type of roms as above */
	/* reads 0x200000 - 0x2fffff ? - 1x socket for larger ROM? */
ROM_END



/*************************
*      Driver Init       *
*************************/

DRIVER_INIT_MEMBER(wildpkr_state,wildpkr)
{
	//HD63484_start(machine());
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT      ROT    COMPANY        FULLNAME                   FLAGS */
GAME( 199?, wildpkr,   0,        wildpkr,  wildpkr, wildpkr_state,  wildpkr,  ROT0, "TAB Austria", "Wild Poker (ver. D 1.01)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 199?, tabpkr,    0,        tabpkr,   wildpkr, wildpkr_state,  wildpkr,  ROT0, "TAB Austria", "Unknown Tab Austria Poker", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
