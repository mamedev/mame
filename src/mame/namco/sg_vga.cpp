// license:BSD-3-Clause
// copyright-holders:

/***********************************************************************************

Namco SH2-based Medal Series, Namco 2000-2001
Hardware info by Guru
---------------------

Games on this system include....
                               Game
Game                Year       Code     Series#
------------------------------------------------
Shooting Paradise   2000       Z029      1st
Galaxian Fever      2000       Z030      2nd
*Pac'n Party        2000       ?         3rd
Happy Planet        2001       Z037      ?

*=confirmed but not dumped.
These games above are all medal shooting games.
There should be several more games, likely some coin pushers and
other redemption games but no others are known or secured yet.
The sound and graphics chips are the same as used on SETA2 hardware.


PCB Layout
----------

SG_VGA MAIN PCB
1828960501 (1828970501)
B0-0018B
(Note later revision 1828960502 (8128970502) B0-0018C just removes some
patch wires but appears identical)

|------------------------------------------------------------|
|    CN12  CN11     CN10  L               |-----|    |---|   |
|               LT1381    L               |X1-010    |   |   |
|                         L  SW01    CN09 |-----|    |   |   |
|CN08         |--------|  L                          |U42|   |
|             |SH2     |  L  7.3728MHz               |   |   |
|             |HD64F7045  L                          |   |   |
|             |F28     |  L       |---|              |   |   |
|       2003  |--U01---|  L       |1016              |---|   |
|                                 |---|     3404         CN07|
|       2003                CN01                     62C256  |
|                                                            | SG_VGA ROM PCB
|     PC410  |-----| |---|                                   | 1828961100 (1828971100) B1-002A
|     PS2801 |NEC  | |   |                                   | |-------------|
|CN06 PS2801 |DX-102 |   |       |-----|            TC551001 | |             |
|            |-----| |U02| DSW01 |NEC  | 70MHz               | |  U01        |
|                    |   |       |DX-102            TC551001 | |             | Notes:
|      TD62083       |   |       |-----|                 CN05| |             |       U01-U04 32Mbit SOP44 mask ROM
|                    |   |                  |-------------|  | |  U03        |       CN01 connects to CN05 on main board
|                    |---| 62C256 62C256    |    NEC      |  | |         CN01|
|CN04       3403           62C256 62C256    |    DX-101   |  | |             |
|                         SW03      60MHz   |             |  | |  U02        |
|                      M1027        |-----| |             |  | |             |
|CN03                               |NEC  | |             |  | |             |
|           LA4705        BATT      |DX102| |-------------|  | |  U04        |
|CN02                               |-----|  62C256 62C256   | |             |
|------------------------------------------------------------| |-------------|
Notes:
      SH2 - Hitachi SH2 HD64F7045F28 QFP144 microcontroller with 256kB internal flash ROM,
            4kB on-chip RAM and on-chip peripherals. Clock input 7.3728MHz.
            This CPU holds the main program. Dumping it is not trivial.
            ROMs:
                 Shooting Paradise - Z029_VER.1.10_SH2_HD64F7045F28.U01
                 Galaxian Fever    - Z030_VER.1.28_SH2_HD64F7045F28.U01
                 Happy Planet      - Z037_VER.1.00_SH2_HD64F7045F28.U01
      U42 - DIP42 32Mbit mask ROM used for audio data. Chip is programmed in BYTE mode.
            ROMs:
                 Shooting Paradise - SPD1_SND-0A.U42
                 Galaxian Fever    - GXF1_SND-0A.U42
                 Happy Planet      - HP1_SND-0A.U42
      U02 - DIP40 MX27C4100 EPROM
            ROMs:
                 Shooting Paradise - SPD1_MPR-0A.U02
                 Galaxian Fever    - GXF1_MPR-0A.U02
                 Happy Planet      - HP1_MPR-0A.U02
     1016 - Lattice iSP1016E QFP44 CPLD, stamped with game code...
             - Shooting Paradise : Z029A
             - Galaxian Fever    : Z030A
             - Happy Planet      : Z037A
   62C256 - ISSI IS62C256 SOJ32 32kBx8-bit Static RAM
 TC551001 - Toshiba TC551001 SOP32 128kBx8-bit Static RAM
        L - LED
     CN01 - Multi-pin expansion connector for a plug-in board (not populated / not used)
     CN02 - 6 pin Namco connector for RGB & Sync Video Output
     CN03 - 4 pin Namco connector, purpose unknown
     CN04 - 9 pin Namco connector for power input
     CN05 - Multi-pin connector for ROM board connection.
            ROM board contains 4x 74HC373 and 4x 64Mbit SOP44 mask ROMs stamped with game code.
            ROMs:
                 Shooting Paradise - SPD1_OBJ-1A.U01  SPD1_OBJ-0A.U02  SPD1_OBJ-3A.U03  SPD1_OBJ-2A.U04
                 Galaxian Fever    - GXF1_OBJ-1A.U01  GXF1_OBJ-0A.U02  GXF1_OBJ-3A.U03  GXF1_OBJ-2A.U04
                 Happy Planet      - HP1_OBJ-1A.U01   HP1_OBJ-0A.U02   HP1_OBJ-3A.U03   HP1_OBJ-2A.U04
     CN06 - 50 pin flat cable connector. Probably used for controls.
     CN07 - Multi-pin connector (not populated / not used)
     CN08 - 20 pin flat cable connector. Probably connected to a drive board for motors and mechanical items in the cabinet.
     CN09 - 8 pin connector joined to Lattice iSP1016 CPLD for JTAG programming.
     CN10 - 25 pin flat cable connector (not populated / not used)
     CN11 - 3 pin connector. This is a serial port TX, RX, GND, most likely for debugging purposes.
            This connector is tied to the LT1381 on pins 7 and 8. Logic-level pins on the LT1381 are tied to the SH2 on pins 130, 131, 133 & 134
     CN12 - 6 pin connector, purpose unknown.
     SW01 - 1 DIP switch with on position tied to SH2 pin 106 (PLLVSS). DIP is off so pin 106 is not tied to GND.
     SW03 - Reset Push Button
    DSW01 - 8 position DIP Switch. Default all off.
   LT1381 - Linear Technology (now Analog Devices) LT1381 Dual RS232 Driver/Receiver Pair
   X1-010 - Allumer/Seta X1-010 Sound Chip
   LA4705 - Sanyo LA4705 15W Stereo Power Amplifier
     3403 - New Japan Radio Co. NJM3403 Quad Operational Amplifier
     3404 - New Japan Radio Co. NJM3404 Dual Operational Amplifier
    M1027 - Mitsumi MM1027 System Reset IC with Battery-Backup Circuit for Static RAM Power Switching
     BATT - 3V Coin Battery
     2003 - ULN2003 Darlington Array
  TD62083 - Toshiba TD62083 8-Channel Darlington Sink Driver
    PC410 - Sharp PC410 Photocoupler
   PS2801 - NEC PS2801-4 Photocoupler
   DX-102 - NEC DX-102 custom chip used for graphics generation
   DX-101 - NEC DX-101 custom chip used for graphics generation

TODO:
- everything, driver-wise
- device-fy DX-101 / DX-102 emulation found in seta/seta2_v.cpp
***********************************************************************************/


#include "emu.h"

#include "cpu/sh/sh7042.h"
#include "sound/x1_010.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sg_vga_state : public driver_device
{
public:
	sg_vga_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sg_vga(machine_config &config) ATTR_COLD;

private:
	required_device<sh7043a_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t sg_vga_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void sg_vga_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x27ffff).rom().region("external_prg", 0);
	map(0x4e0000, 0x4effff).ram();
	map(0x800000, 0x83ffff).ram().share("spriteram"); // ?
	map(0x840000, 0x84ffff).ram().share("palette"); // ?
	map(0x860000, 0x86003f).ram().share("vregs"); // ?
}


static INPUT_PORTS_START( hplanet )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW01:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(7*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};


/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( gfx_dx_10x )
	GFXDECODE_ENTRY( "sprites", 0, tile_layout, 0, 0x8000 / 16 )   // 8bpp, but 4bpp color granularity
GFXDECODE_END


void sg_vga_state::sg_vga(machine_config &config)
{
	// basic machine hardware
	SH7043A(config, m_maincpu, 7.3728_MHz_XTAL * 4); // actually SH7045
	m_maincpu->set_addrmap(AS_PROGRAM, &sg_vga_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(0x200, 0x100);
	screen.set_visarea(0x00, 0x140-1, 0x00, 0xe0-1);
	screen.set_screen_update(FUNC(sg_vga_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_dx_10x);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x8000 + 0xf0); // extra 0xf0 because we might draw 256-color object with 16-color granularity

	// sound hardware
	SPEAKER(config, "mono").front_center();

	x1_010_device &x1snd(X1_010(config, "x1snd", 16'000'000)); // clock unknown
	x1snd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( hplanet )
	ROM_REGION( 0x40000, "maincpu", 0 ) // internal ROM
	ROM_LOAD( "z037_ver.1.00_sh2_hd64f7045f28.u01", 0x00000, 0x40000, CRC(5b2e5b30) SHA1(b69b9a0cc70816449c8894fd6a46ce852b9883ab) )

	ROM_REGION32_BE( 0x80000, "external_prg", 0 )
	ROM_LOAD16_WORD_SWAP( "hp1_mpr-0a.u02", 0x00000, 0x80000, CRC(83dd903f) SHA1(37ca41f8423bcf6e7ae422a5a6e1aef0e52a38cc) )

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD64_WORD( "hp1_obj-1a.u01", 0x000000, 0x800000, CRC(124f99b9) SHA1(41970a06a95875688f8bc21465a5bbd80b8cf8ce) )
	ROM_LOAD64_WORD( "hp1_obj-0a.u02", 0x000002, 0x800000, CRC(597e179c) SHA1(5130c5f4f7f8b3c730363ff843440fa5f059663c) )
	ROM_LOAD64_WORD( "hp1_obj-3a.u03", 0x000004, 0x800000, CRC(17eeb4fd) SHA1(3bc30cd8f6a43d4aee9cd2da119dbab66c99565e) )
	ROM_LOAD64_WORD( "hp1_obj-2a.u04", 0x000006, 0x800000, CRC(31f71432) SHA1(b572045af0c0ad54df72d9396168be004c07f7f7) )

	ROM_REGION( 0x400000, "x1snd", 0 )
	ROM_LOAD( "hp1_snd-0a.u42", 0x000000, 0x400000, CRC(a78b01e5) SHA1(20e904a2e01a7e40c037a7c4ab9bd1b4e9054c4d) )
ROM_END

} // anonymous namespace


GAME( 2001, hplanet, 0, sg_vga, hplanet, sg_vga_state, empty_init, ROT0, "Namco", "Happy Planet", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
