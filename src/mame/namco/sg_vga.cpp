// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

TODO:
- Has background GFX cutoffs (gameplay and service mode);
- LED N/G;
- complete I/O;
- Allegedly should have language select somewhere (Chinese, Korean, English, Japanese);

===================================================================================================

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
   DX-102 - NEC DX-102 custom chip (unknown usage)
   DX-101 - NEC DX-101 custom chip used for graphics generation

**************************************************************************************************/


#include "emu.h"

#include "cpu/sh/sh7042.h"
#include "sound/x1_010.h"
#include "video/x1_020_dx_101.h"

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
		m_maincpu(*this, "maincpu"),
		m_video(*this, "video"),
		m_in(*this, "IN%u", 0U),
		m_medal(*this, "MEDAL%u", 0U)
	{ }

	void sg_vga(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<sh7043a_device> m_maincpu;
	required_device<x1_020_dx_101_device> m_video;
	required_ioport_array<6> m_in;
	required_ioport_array<8> m_medal;

	void program_map(address_map &map) ATTR_COLD;

	void pd_w(u32 data);
	u16 pf_r();

	u8 m_medal_select;
};

void sg_vga_state::machine_start()
{
	save_item(NAME(m_medal_select));
}

void sg_vga_state::machine_reset()
{
	m_medal_select = 0;
}

void sg_vga_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x27ffff).rom().region("external_prg", 0);
	// initializes in $42xxxx, reads in $43xxxx in "backup ram test"
	map(0x420000, 0x427fff).mirror(0x8000).ram().share("nvram");
	map(0x430000, 0x437fff).mirror(0x8000).ram().share("nvram");
	map(0x440000, 0x440003).lr32(
		NAME([this] () { return 0xff00ff00 | (m_in[1]->read() << 16) | m_in[0]->read(); })
	);
	// RMW, unknown purpose
//	map(0x440004, 0x440007).lr32(NAME([] () { return 0xffff'ffff; }));
	map(0x460000, 0x460003).lr32(
		NAME([this] () { return 0xff00ff00 | (m_in[3]->read() << 16) | m_in[2]->read(); })
	);
	map(0x460004, 0x460007).lr32(
		NAME([this] () { return 0xff00ff00 | (m_in[5]->read() << 16) | m_in[4]->read(); })
	);
	map(0x4e0000, 0x4effff).ram();
	map(0x500000, 0x503fff).rw("x1snd", FUNC(x1_010_device::word_r), FUNC(x1_010_device::word_w));
	map(0x800000, 0x83ffff).rw(m_video, FUNC(x1_020_dx_101_device::spriteram_r), FUNC(x1_020_dx_101_device::spriteram_w));
	map(0x840000, 0x84ffff).ram().w("palette", FUNC(palette_device::write32)).share("palette");
	map(0x860000, 0x86003f).rw(m_video, FUNC(x1_020_dx_101_device::vregs_r), FUNC(x1_020_dx_101_device::vregs_w));
}

// bit 30: always active
// bits 20-17: medal input select
// everything else: unused?
void sg_vga_state::pd_w(u32 data)
{
//	if (!BIT(data, 30) || data & 0xbff1'ffff)
//		printf("%08x\n", data);
	m_medal_select = (data & 0x000e'0000) >> 17;
}

u16 sg_vga_state::pf_r()
{
	return m_medal[m_medal_select]->read();
}

static INPUT_PORTS_START( hplanet )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" ) // used in sound test (?)
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // analyzer, needs being held (use PORT_TOGGLE to test medal gun sensor)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("White Button (Choose)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red Button (Enter)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gun") // TODO: intelligible
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) // hopper
	PORT_DIPNAME( 0x20, 0x20, "IN1" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) // ticket

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) // battery NG if low
	PORT_DIPNAME( 0x02, 0x02, "IN2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3-DSW" ) PORT_DIPLOCATION("DSW01:1")
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
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "DSW01:8" )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MEDAL0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("MEDAL1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MEDAL2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MEDAL3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 04")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 09")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 14")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 19")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 24")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 29")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 34")

	PORT_START("MEDAL4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 05")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 15")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 20")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 25")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 30")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 35")

	PORT_START("MEDAL5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 01")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 06")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 16")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 26")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 36")

	PORT_START("MEDAL6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 02")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 07")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 17")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 22")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 27")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 32")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 37")

	PORT_START("MEDAL7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 03")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 08")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 13")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 18")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 23")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 28")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Medal 33")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// 3 irqs:
// 0 vblank
// 1 may be raster irq like other Namco games (unused?)
// 2 unknown
void sg_vga_state::sg_vga(machine_config &config)
{
	// basic machine hardware
	SH7043A(config, m_maincpu, 7.3728_MHz_XTAL * 4); // actually SH7045
	m_maincpu->set_addrmap(AS_PROGRAM, &sg_vga_state::program_map);
	m_maincpu->read_portd().set_constant(0);
	m_maincpu->write_portd().set(FUNC(sg_vga_state::pd_w));
//	m_maincpu->read_porte().set(FUNC(sg_vga_state::pe_r));
//	m_maincpu->write_porte().set(FUNC(sg_vga_state::pe_w));
	m_maincpu->read_portf().set(FUNC(sg_vga_state::pf_r));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(0x25a, 0x1b6); // 0x2c 0x58 0x22a 0x25a / 0x06 0x21 0x191 0x1b6
	screen.set_visarea(0x00, 0x1d2-1, 0x00, 0x170-1);
	screen.set_screen_update(m_video, FUNC(x1_020_dx_101_device::screen_update));
	screen.screen_vblank().set(m_video, FUNC(x1_020_dx_101_device::screen_vblank));
	screen.screen_vblank().append_inputline(m_maincpu, 0);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x8000 + 0xf0); // extra 0xf0 because we might draw 256-color object with 16-color granularity

	X1_020_DX_101(config, m_video, XTAL(60'000'000)); // or 70MHz? unverified
	m_video->set_palette("palette");
	m_video->set_screen("screen");
	//m_video->raster_irq_callback().set_inputline(m_maincpu, 1, HOLD_LINE);
	m_video->flip_screen_callback().set(FUNC(sg_vga_state::flip_screen_set));
	m_video->flip_screen_x_callback().set(FUNC(sg_vga_state::flip_screen_x_set));
	m_video->flip_screen_y_callback().set(FUNC(sg_vga_state::flip_screen_y_set));

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

	ROM_REGION( 0x2000000, "video", 0 )
	ROM_LOAD64_WORD( "hp1_obj-1a.u01", 0x000002, 0x800000, CRC(124f99b9) SHA1(41970a06a95875688f8bc21465a5bbd80b8cf8ce) )
	ROM_LOAD64_WORD( "hp1_obj-0a.u02", 0x000000, 0x800000, CRC(597e179c) SHA1(5130c5f4f7f8b3c730363ff843440fa5f059663c) )
	ROM_LOAD64_WORD( "hp1_obj-3a.u03", 0x000006, 0x800000, CRC(17eeb4fd) SHA1(3bc30cd8f6a43d4aee9cd2da119dbab66c99565e) )
	ROM_LOAD64_WORD( "hp1_obj-2a.u04", 0x000004, 0x800000, CRC(31f71432) SHA1(b572045af0c0ad54df72d9396168be004c07f7f7) )

	ROM_REGION( 0x400000, "x1snd", 0 ) // TODO: bankswitched?
	ROM_LOAD( "hp1_snd-0a.u42", 0x000000, 0x400000, CRC(a78b01e5) SHA1(20e904a2e01a7e40c037a7c4ab9bd1b4e9054c4d) )
ROM_END

} // anonymous namespace


GAME( 2001, hplanet, 0, sg_vga, hplanet, sg_vga_state, empty_init, ROT0, "Namco", "Happy Planet (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
