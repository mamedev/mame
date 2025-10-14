// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Angelo Salese
/******************************************************************************

  COINMASTER-GAMING - VGA BOARD
  -----------------------------

  Preliminary driver by Roberto Fresca & Angelo Salese.


  Games running on this hardware:

  * Colorama (standalone),   199x-2001,  Coinmaster-Gaming, Ltd.
  * Roulette (bet station).  199x-2001,  Coinmaster-Gaming, Ltd.
  * Keno, (bet station).     2000-2001,  Coinmaster-Gaming, Ltd.


*******************************************************************************


  Hardware Notes:
  ---------------

  This board is used in each bet station of Coinmaster's Roulette and Keno games.
  Both systems have a physical electromechanical unit with their own controller
  plus sound. The central units (wheel controller) are routed to the bet stations
  (10 for default) through 2 different kind of networks, depending the wheel system.

  There are 2 multisystems types:


  - System RWC (Roulette Wheel Controller)

  The old version. The central unit is tied to a controller and then routed to
  the bet stations using a custom serial network with RJ9 connectors and regular
  phone cable. Also has a thermal printer attached.


  - System Y2K (Year 2000)

  The newer system. The central unit has 4 trays inside with power supply, roulette
  controller, and a custom HUB board. All the system is interconnected through
  regular RJ45 CAT5 cables from the hub board. Also has a thermal printer attached.


  All systems are controlled through a touch screen.


  Colorama is a standalone roulette system, driven by one VGA board. It has a big lamps
  loom, and a vertical positioned roulette painted on the marquee with one physical arm
  pointing to the numbers.



  Bet station (VGA board):
  -----------------------

  PCB-VGA-001 R0
  COINMASTER MANUFACTURING LIMITED (C)1997.

  1x H8/3002 (HD6413002F16) or H8/3007 (HD6413007F20) CPU (IC1)
  1x AMD MACH 131-15JC CPLD.   (IC29)
  1x AMD MACH 231-7JC CPLD.    (IC12)
  1x ADV471.                   (IC30)

  1x YMZ280B (sound).          (IC33)

  1x PALCE 22V10H25 PC/4.      (IC11)
  1x MSM62X42B.                (IC6)
  1x SMC COM20020I-P (ARCNET controller) (IC31)

  2x K6T10082CE-DB70.          (IC4, IC5)
  2x HY62256BLP-70.            (IC13, IC14)

  1x 14.7456 MHz.Xtal (H8 CPU direct clock).
  1x 20.0000 MHz.Xtal (COM20020 direct clock).
  1x 50.35 Mhz module (MACH 231 direct clock).
  1x 16.9344 MHz.Xtal (YMZ280B direct? clock).

  2x 27c040 --> Program.        (IC2, IC3)
  2x 27c040 --> Foreground GFX. (IC23, IC24)
  4x 27c801 --> Background GFX. (IC25, IC26, IC27, IC28)
  1x 27c801 --> Sound.

  1x 8 DIP switches bank.
  1x 15-pin VGA connector.
  1x Microtouch connector.



  The VGA board is a general-purpose control board with the following features:

  * 24 discrete inputs (pulled up on board).
  * 32 discrete open collector Darlington drivers.
  * 2 RS232 channels.
  * Stereo, digital sound replay and amplification.
  * Arcnet network communications interface.
  * VGA video graphics generation
  * Power supply monitoring and static (spark) discharge detector.
  * Battery backed up RAM


  Input | Pin | Colours | Ground
  ------+-----+---------+---------------------------------------------------------------
  00    | 1A  | Brn-Wht | Universal Hopper Early / AWP hopper switch 1 (?1) / Y2K Hop 1.
  01    | 2A  | Red-Wht | Universal Hopper Late / AWP hopper switch 2 (20p) / Y2K Hop 2.
  02    | 3A  | Org-Wht | CC46 Coin / C435 Accept 1.
  03    | 4A  | Yel-Wht | CC46 Error / C435 Accept 2.
  04    | 5A  | Grn-Wht | CC46 Sensor / C435 Accept 3.
  05    | 6A  | Blu-Wht | Lid Switch.
  06    | 7A  | Vio-Wht | Door Switch.
  07    | 8A  | Gry-Wht | Key Switch.
  08    | 17A | Brn-Pnk | Shooting Button / Start Button.
  09    | 18A | Red-Pnk | C435 Accept 4.
  10    | 19A | Org-Pnk | Dumbcard Present.
  11    | 20A | Yel-Pnk | Small Hopper Return / Y2K Hop 3.
  12    | 21A | Grn-Pnk | Dumbcard / Bill.
  13    | 22A | Blu-Pnk | C435 Accept 5.
  14    | 23A | Vio-Pnk | Cancel Button.
  15    | 24A | Gry-Pnk | Payout Button.
  16    | 27A | Red-Brn | Gamble Button.
  17    | 28A | Org-Brn |
  18    | 29A | Yel-Brn |
  19    | 30A | Grn-Brn |
  20    | 31A | Blu-Brn |
  21    | 32A | Vio-Brn |
  22    | 33A | Gry-Brn |
  23    | 34A | Pnk-Brn |


  Output | Pin | Colours | Description
  -------+-----+---------+---------------------------------------------------------------
  00     | 4B  | Brn-Blu | Universal Hopper Drive / AWP hopper drive 1 (?1) / Y2K Hop 1.
  01     | 5B  | Red-Blu | Candle 1.
  02     | 6B  | Org-Blu | Candle 2.
  03     | 7B  | Yel-Blu | Cancel Button Lamp.
  04     | 8B  | Grn-Blu | CC46 Deflector / C435 Inhibit 1.
  05     | 1B  | Vio-Blu | CC46 Inhibit / C435 Inhibit 2.
  06     | 2B  | Gry-Blu | C435 Inhibit 3.
  07     | 3B  | Wht-Blu | C435 Inhibit 4.
  08     | 10B | Brn-Vio | Small Hopper Drive / AWP hopper drive 2 (20p) / Y2K Hop 2.
  09     | 11B | Red-Vio | Shooting Button Lamp / Start Button Lamp.
  10     | 12B | Org-Vio | Payout Button Lamp.
  11     | 13B | Yel-Vio | Gamble Button Lamp.
  12     | 14B | Vio-Red | Y2K Hop 3.
  13     | 15B | Blu-Vio | Candle 3.
  14     | 16B | Gry-Vio | C435 Overide B.
  15     | 17B | Wht-Vio | C435 Overide C.
  16     | 24B | Brn-Gry | C435 Inhibit 5.
  17     | 25B | Red-Gry | C435 Inhibit 6.
  18     | 26B | Org-Gry | C435 Inhibit 7.
  19     | 27B | Yel-Gry | C435 Inhibit 8.
  20     | 28B | Grn-Gry | C435 Overide D.
  21     | 29B | Blu-Gry |
  22     | 30B | Vio-Gry | Dumbcard Trigger.
  23     | 31B | Wht-Gry | Screen Supply.
  24     | 38B | Org-Red | Credit In (Coin In).
  25     | 39B | Yel-Red | Credit Out (Coin Out).
  26     | 40B | Grn-Red | Cashbox (Cashbox).
  27     | 37B | Blu-Red | Remote Out (Canceled Credits).
  28     | 36B | Vio-Red | Games.
  29     | 35B | Gry-Red | Tot Wins.
  30     | 34B | Wht-Red | Total Bet.
  31     | 33B | Pnk-Red | Jackpot.


*******************************************************************************


  *** Games Notes ***


  Nothing, yet...


*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x000000 - 0x09FFFF    ; ROM space.
  0x210000 - 0x21FFFF    ; NVRAM?.


*******************************************************************************


  DRIVER UPDATES:


  [2014-08-15]

  - Added Colorama (P521 V13, Spanish).
  - Changed the Colorama parent set description to Colorama (P521, English).
  - Added technical notes.


  [2009-08-18]

  - Renamed Roulette V75 to Coinmaster Roulette V75.
  - Added Roulette controller program & sound ROMs.
  - Added 2 complete spanish Keno sets.
  - Added technical notes.


  [2009-08-17]

  - Initial release.
  - Added technical notes.


  TODO:

  - Microtouch touch screen hook-up;
  - cmkenosp: corrupts the RAMDAC palette during POST;
  - cmkenosp/cmkenospa: doesn't draw foreground tiles properly;

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h83002.h"
//#include "cpu/h8/h83006.h"
#include "sound/ymz280b.h"
#include "machine/i2cmem.h"
#include "machine/msm6242.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define CPU_CLOCK   XTAL(14'745'600)
#define MACH_CLOCK  XTAL(50'350'000)
#define COM_CLOCK   XTAL(20'000'000)
#define SND_CLOCK   XTAL(16'934'400)


class coinmvga_state : public driver_device
{
public:
	coinmvga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "vram")
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_gfxdecode(*this, "gfxdecode%u", 0U)
		, m_palette(*this, "palette%u", 0U)
	{ }

	void coinmvga(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t i2c_r();
	void i2c_w(uint8_t data);

	uint32_t screen_update_coinmvga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_shared_ptr<uint16_t> m_vram;
	required_device<cpu_device> m_maincpu;
	required_device<i2cmem_device> m_eeprom;
	required_device_array<gfxdecode_device, 2> m_gfxdecode;
	required_device_array<palette_device, 2> m_palette;

	void coinmvga_map(address_map &map) ATTR_COLD;
	void ramdac2_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


uint8_t coinmvga_state::i2c_r()
{
	return 0x04 | m_eeprom->read_sda() << 1;
}

void coinmvga_state::i2c_w(uint8_t data)
{
	m_eeprom->write_sda(BIT(data, 1));
	m_eeprom->write_scl(BIT(data, 2));
}


/*************************
*     Video Hardware     *
*************************/


void coinmvga_state::video_start()
{
}


uint32_t coinmvga_state::screen_update_coinmvga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx_8bpp = m_gfxdecode[0]->gfx(0);
	gfx_element *gfx_4bpp = m_gfxdecode[1]->gfx(0);

	for (int y = 0; y < 64; y++)
	{
		const u16 y_offs = y * 128;
		for (int x = 0; x < 128; x++)
		{
			const u16 tile_offset = y_offs + x;

			// background layer
			u16 tile = (m_vram[tile_offset] & 0xffff);
			//int colour = tile>>12;
			gfx_8bpp->opaque(bitmap, cliprect, tile, 0, 0, 0, x*8, y*8);

			// foreground layer
			tile = (m_vram[tile_offset + (0x4000/2)] & 0xffff);
			//int colour = tile>>12;
			gfx_4bpp->transpen(bitmap, cliprect, tile, 0, 0, 0, x*8, y*8, 0);
		}
	}


	return 0;
}


/**************************
*  Read / Write Handlers  *
**************************/

//void coinmvga_state::debug_w(uint8_t data)
//{
//  popmessage("written : %02X", data);
//}

/*
uint16_t coinmvga_state::test_r()
{
    return machine().rand();
}*/

/*************************
* Memory Map Information *
*************************/

void coinmvga_state::coinmvga_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x0fffff).rom().region("maincpu", 0); //maybe not

//  map(0x0a0000, 0x0fffff).ram();
//  map(0x100000, 0x1fffff).ram(); //colorama
	map(0x210000, 0x21ffff).ram().share("vram");
//  map(0x40746e, 0x40746f).r(FUNC(coinmvga_state::test_r)).nopw(); //touch screen related, colorama
//  map(0x403afa, 0x403afb).r(FUNC(coinmvga_state::test_r)).nopw(); //touch screen related, cmrltv75
	map(0x400000, 0x40ffff).ram();

	map(0x600000, 0x600000).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x600001, 0x600001).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x600002, 0x600002).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x600004, 0x600004).w("ramdac2", FUNC(ramdac_device::index_w));
	map(0x600005, 0x600005).w("ramdac2", FUNC(ramdac_device::pal_w));
	map(0x600006, 0x600006).w("ramdac2", FUNC(ramdac_device::mask_w));
	map(0x600008, 0x600009).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x610000, 0x61000f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));

	map(0x700000, 0x7fffff).rom().region("maincpu", 0); // ?

	map(0x800000, 0x800001).portr("DSW1"); //"arrow" r?
	map(0x800002, 0x800003).portr("DSW2");
	//0x800008 "arrow" w?
}

/*  Digital I/O ports (ports 4-B are valid on 16-bit H8/3xx) */
//  map(h8_device::PORT_4, h8_device::PORT_4)
//  map(h8_device::PORT_5, h8_device::PORT_5)
//  map(h8_device::PORT_6, h8_device::PORT_6)
//  map(h8_device::PORT_7, h8_device::PORT_7) <---- 0006 RW colorama
//  map(h8_device::PORT_8, h8_device::PORT_8)
//  map(h8_device::PORT_9, h8_device::PORT_9)
//  map(h8_device::PORT_A, h8_device::PORT_A)
//  map(h8_device::PORT_B, h8_device::PORT_B)

/*  Analog Inputs */
//  map(h8_device::ADC_0, h8_device::ADC_0)
//  map(h8_device::ADC_1, h8_device::ADC_1)
//  map(h8_device::ADC_2, h8_device::ADC_2)
//  map(h8_device::ADC_3, h8_device::ADC_3)

/*  unknown writes (cmrltv75):

CPU 'main' (PC=00000218): unmapped program memory word write to 00FFFFF2 = 001E & 00FF
CPU 'main' (PC=0000021C): unmapped program memory word write to 00FFFFEC = E800 & FF00
CPU 'main' (PC=00000220): unmapped program memory word write to 00FFFFEC = 00FF & 00FF
CPU 'main' (PC=00000224): unmapped program memory word write to 00FFFFEE = 00FD & 00FF
CPU 'main' (PC=00000228): unmapped program memory word write to 00FFFFEE = F300 & FF00
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040FBF2 = 0000 & FFFF
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040FBF4 = 023E & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040FBEE = 0000 & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040FBF0 = 0254 & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040FBEE & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040FBF0 & FFFF


**  unknown writes (colorama):


CPU 'main' (PC=00000218): unmapped program memory word write to 00FFFFF2 = 001E & 00FF
CPU 'main' (PC=0000021C): unmapped program memory word write to 00FFFFEC = E800 & FF00
CPU 'main' (PC=00000220): unmapped program memory word write to 00FFFFEC = 00FF & 00FF
CPU 'main' (PC=00000224): unmapped program memory word write to 00FFFFEE = 00FD & 00FF
CPU 'main' (PC=00000228): unmapped program memory word write to 00FFFFEE = F300 & FF00
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040B572 = 0000 & FFFF
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040B574 = 023E & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040B56E = 0000 & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040B570 = 0254 & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040B56E & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040B570 & FFFF
CPU 'main' (PC=000000D4): unmapped program memory word write to 0040B5FA = 0000 & FFFF
CPU 'main' (PC=000000D4): unmapped program memory word write to 0040B5FC = 00D6 & FFFF


000214: F81E    ;mov.b #1e, R0L
000216: 38F3    ;mov.b R0L, @0ffffff3
000218: F8E8    ;mov.b #e8, R0L
00021A: 38EC    ;mov.b R0L, @0fffffec
00021C: F8FF    ;mov.b #ff, R0L
00021E: 38ED    ;mov.b R0L, @0fffffed
000220: F8FD    ;mov.b #fd, R0L
000222: 38EF    ;mov.b R0L, @0fffffef
000224: F8F3    ;mov.b #f3, R0L
000226: 38EE    ;mov.b R0L, @0fffffee
000228: F8F8    ;mov.b #f8, R0L
00022A: 38CD    ;mov.b R0L, @0fffffcd

600000 W \
600002 W  | seems registers...
600004 W  |
600008 W /

210000-21ffff W (NVRAM?)


I/O

0006 RW


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( coinmvga )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-0001")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN0-0002")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN0-0004")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN0-0008")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN0-0010")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN0-0020")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN0-0040")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN0-0080")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN0-0100")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN0-0200")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN0-0400")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN0-0800")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN0-1000")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN0-2000")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN0-4000")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN0-8000")

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN1-0001")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN1-0002")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN1-0004")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN1-0008")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN1-0010")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN1-0020")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN1-0040")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN1-0080")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN1-0100")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN1-0200")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN1-0400")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN1-0800")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN1-1000")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN1-2000")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN1-4000")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN1-8000")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tiles8x8x4bpp_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 1) },
	{ 12, 8, 4, 0, 28, 24, 20, 16  },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};

// TODO: convert to ROM_LOAD32_BYTE
// cfr. cmkenospa offset $0 charset
static const gfx_layout tiles8x8x8bpp_layout =
{
	8, 8,
	RGN_FRAC(1, 2),
	8,
	{ STEP8(0, 1) },
	{ RGN_FRAC(1, 2) +  0, RGN_FRAC(1, 2) +  8,  0,  8,
	  RGN_FRAC(1, 2) + 16, RGN_FRAC(1, 2) + 24, 16, 24 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};

/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_coinmvga_4bpp )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4bpp_layout, 0x000, 1 )  // Foreground GFX
GFXDECODE_END

static GFXDECODE_START( gfx_coinmvga_8bpp )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8x8bpp_layout, 0x000, 1 )  // Background GFX
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void coinmvga_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void coinmvga_state::ramdac2_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac2", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void coinmvga_state::coinmvga(machine_config &config)
{
	// basic machine hardware
	// could be either H8/3002 or H8/3007
//  H83007(config, m_maincpu, CPU_CLOCK);
	h83002_device &maincpu(H83002(config, m_maincpu, CPU_CLOCK));
	maincpu.set_addrmap(AS_PROGRAM, &coinmvga_state::coinmvga_map);
	maincpu.read_port6().set(FUNC(coinmvga_state::i2c_r));
	maincpu.write_port6().set(FUNC(coinmvga_state::i2c_w));

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I2C_24C04(config, "eeprom");

	MSM6242(config, "rtc", 32768);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MACH_CLOCK / 2, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(FUNC(coinmvga_state::screen_update_coinmvga));
	//screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_coinmvga_8bpp);
	GFXDECODE(config, m_gfxdecode[1], m_palette[1], gfx_coinmvga_4bpp);

	PALETTE(config, m_palette[0]).set_entries(256);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette[0]));
	ramdac.set_addrmap(0, &coinmvga_state::ramdac_map);

	PALETTE(config, m_palette[1]).set_entries(16);
	ramdac_device &ramdac2(RAMDAC(config, "ramdac2", 0, m_palette[1]));
	ramdac2.set_addrmap(0, &coinmvga_state::ramdac2_map);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ymz280b_device &ymz(YMZ280B(config, "ymz", SND_CLOCK));
	ymz.irq_handler().set_inputline("maincpu", 2);
	ymz.add_route(0, "speaker", 1.0, 0);
	ymz.add_route(1, "speaker", 1.0, 1);
}


/*************************
*        Rom Load        *
*************************/

/*
   Colorama.
   p521 (unknown version), English.

   Standalone. Physical arm on marquee + bet station.
*/
ROM_START( colorama )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p521_prg1.cp1", 0x00001, 0x80000, CRC(f5da12cb) SHA1(d75fdda09e57c8a4637b0bcc98931796b5449435) )
	ROM_LOAD16_BYTE( "p521_prg2.cp2", 0x00000, 0x80000, CRC(6db85d66) SHA1(21009aa01db5193d1be588deaeba8f89582d53dd) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p521_fore1.fg1",  0x00001, 0x80000, CRC(0a8fe27a) SHA1(29500040e2bd0b6f349abaf51bb7f8aaac73e8cf) )
	ROM_LOAD16_BYTE( "p521_fore2.fg2",  0x00000, 0x80000, CRC(3ae78445) SHA1(ef590a6042969718d88732244d2639b7cd8ab507) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p521_back1.bg1",  0x200000, 0x100000, CRC(0c1a7a2d) SHA1(a7827c6091d0f78e146419261eca427cd229d445) )
	ROM_LOAD16_BYTE( "p521_back2.bg2",  0x200001, 0x100000, CRC(218912d7) SHA1(64e3dc22ff6ae296e1843b6d6bfb02eb0d202db5) )
	ROM_LOAD16_BYTE( "p521_back3.bg3",  0x000000, 0x100000, CRC(8ddad7d1) SHA1(0a41ca166c8a9eca2ee27d35a3ae41ddb8759dce) )
	ROM_LOAD16_BYTE( "p521_back4.bg4",  0x000001, 0x100000, CRC(28d54ce1) SHA1(0dadae2e11f9b86dddb6a0c33abfbdb8b6f2d862) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "p521_snd.sp1",   0x00000, 0x100000, CRC(5c87bb98) SHA1(bc1b8c090fbae166e3a7e1da74bfd2e84c1a03f6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )

ROM_END

/*
   Colorama.
   p521 v13, Spanish.

   Standalone. Physical arm on marquee + bet station.
*/
ROM_START( coloramas )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p521_v13_rwof_prog_1_=401=_14-2-00_spanish.bin",  0x00001, 0x80000, CRC(69c26df0) SHA1(a83232e835a24e4da46a613abfa34ca2440727ac) )
	ROM_LOAD16_BYTE( "p521_v13_rwof_prog_2_=401=_14-2-00_spanish.bin",  0x00000, 0x80000, CRC(42294c43) SHA1(f8a94d0387eb2f58643570017499c70baaa393cc) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p521_v12_rwof_fore_1_=401=_20-7-99_spanish.bin",  0x00001, 0x80000, CRC(c5187559) SHA1(a32cee8948eb08fa9662622164f7ba9042d297d8) )
	ROM_LOAD16_BYTE( "p521_v12_rwof_fore_2_=401=_20-7-99_spanish.bin",  0x00000, 0x80000, CRC(fdf71c26) SHA1(4e2e5cc3f847a173283401969e21ccde941f0f20) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p521_v12_rwof_back_1_=801=_20-7-99_spanish.bin",  0x200000, 0x100000, CRC(0cbaf152) SHA1(2d6dfc7a4a8ccb6891dd8859594711ddf8a1055e) )
	ROM_LOAD16_BYTE( "p521_v12_rwof_back_2_=801=_20-7-99_spanish.bin",  0x200001, 0x100000, CRC(7e840b74) SHA1(3825533a824a9a47e4bd44adcebbdc56a01a6f1e) )
	ROM_LOAD16_BYTE( "p521_v12_rwof_back_3_=801=_20-7-99_spanish.bin",  0x000000, 0x100000, CRC(3163f25d) SHA1(ea2336f2381de1680046c70f217c398d1229f11f) )
	ROM_LOAD16_BYTE( "p521_v12_rwof_back_4_=801=_20-7-99_spanish.bin",  0x000001, 0x100000, CRC(e741a046) SHA1(8b65205c1d55dfca953e3626d151cb28ba1b2dfc) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "p521_v12_rwof_bet_sound_=801=_20-7-99_spanish.bin",  0x00000, 0x100000, CRC(a9bda811) SHA1(c5a9aa83bba4bed00f4b23f17b82100c94e2889c) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )
ROM_END

/*
   Wheel of Fortune.
   p517 (v11?).

   Standalone. Physical arm on marquee + bet station.
*/
ROM_START( wof_v11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p517v11_wof.pr1", 0x00001, 0x80000, CRC(d00ab658) SHA1(1014807a7f16dc0de7a2136940c437c87f7e8e42) )
	ROM_LOAD16_BYTE( "p517v11_wof.pr2", 0x00000, 0x80000, CRC(c8d6eabf) SHA1(a3b830a1e72d1e60762cb52c8b9f55d669f8cdba) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p517v11_wof.fg1",  0x00001, 0x80000, CRC(6a16849d) SHA1(95d67f03e203a967366eec171daeaaa9b430de5b) )
	ROM_LOAD16_BYTE( "p517v11_wof.fg2",  0x00000, 0x80000, CRC(5f3e0c8b) SHA1(7d3b36f106c7fba322ac910a44b1f4ee0e0eb1a8) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p517v11_wof.bg1",  0x200000, 0x100000, CRC(02af35f0) SHA1(c064c0cd30c59b36b4dcc20e5a196854e4cbe22f) )
	ROM_LOAD16_BYTE( "p517v11_wof.bg2",  0x200001, 0x100000, CRC(d8bb7737) SHA1(c5666d4679867c69ef62253f71fac21b2e07f448) )
	ROM_LOAD16_BYTE( "p517v11_wof.bg3",  0x000000, 0x100000, CRC(a9e42c43) SHA1(27e8ba0a762f44ebece793e485bdb2d36f6df2fa) )
	ROM_LOAD16_BYTE( "p517v11_wof.bg4",  0x000001, 0x100000, CRC(1ddabd6f) SHA1(3095d947c8b1ff5e547a7c889a1e27bf6b5a6704) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "p517v11_wof.snd",   0x00000, 0x100000, CRC(6d90f7ca) SHA1(5360331ce8de74c71149480a345586145bddbca2) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )

ROM_END

/*
   Wheel of Fortune.
   p517 (v16?).
   Standalone. Physical arm on marquee + bet station.

   Program ROMs stickers have the version illegible.
   The plus sign means minor revision.

*/
ROM_START( wof_v16 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p517vxx+_wof.pr1", 0x00001, 0x80000, CRC(1b3f17d7) SHA1(57f5aabdd065546a9bf6ebea0d74350b8ef3e44e) )
	ROM_LOAD16_BYTE( "p517vxx+_wof.pr2", 0x00000, 0x80000, CRC(a40467db) SHA1(0e74ef023a8aeb4f5bbd3dbe5cab4d9de19c6f10) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p517v16_wof.fg1",  0x00001, 0x80000, CRC(61d15d45) SHA1(9ed8abca13b9494151b5b06fe20111d0863ce4b0) )
	ROM_LOAD16_BYTE( "p517v16_wof.fg2",  0x00000, 0x80000, CRC(9968ac3a) SHA1(b3c78ea795a6350b3abff54ebbbeff6432dd16fc) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p517v16_wof.bg1",  0x200000, 0x100000, CRC(39f023b5) SHA1(4cd855683226697767eb76a24ef0535950c6d663) )
	ROM_LOAD16_BYTE( "p517v16_wof.bg2",  0x200001, 0x100000, CRC(40b6f0ad) SHA1(b5e94c91a2ea1d240f3710955f72d5f832b853b6) )
	ROM_LOAD16_BYTE( "p517v16_wof.bg3",  0x000000, 0x100000, CRC(4ebc7f22) SHA1(4717bd4f88024ae11d87514a8e3a2c489abfb149) )
	ROM_LOAD16_BYTE( "p517v16_wof.bg4",  0x000001, 0x100000, CRC(1de571c9) SHA1(df7de4e985ff39977f0553d3f8a588e91d003d00) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "p517v16_wof.snd",   0x00000, 0x100000, CRC(6d90f7ca) SHA1(5360331ce8de74c71149480a345586145bddbca2) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )

ROM_END


/*
   Coinmaster Roulette V75 (y2k, spanish)
   Physical Unit + 10-15 bet stations.
*/
ROM_START( cmrltv75 )
	/*** Bet Station ***/
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bet.cp1",   0x00001, 0x80000, CRC(2dc1c899) SHA1(2be488d23df5e50bbcfa4e66a49a455c617b29b4) )
	ROM_LOAD16_BYTE( "bet.cp2",   0x00000, 0x80000, CRC(fcab8825) SHA1(79cb862ac5363ab90e91184efd9cfaec86bb82a5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p497.fg1",    0x00001, 0x80000, CRC(ce5f9fe9) SHA1(a30f5f375eaa651ede4057449c1648c64d207577) )
	ROM_LOAD16_BYTE( "p497.fg2",    0x00000, 0x80000, CRC(3846fad0) SHA1(409725ab8c9353a8d5774c5f010ace1077b3fd35) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p497.bg1",    0x200000, 0x100000, CRC(fadf2a5a) SHA1(ac5413ff213ef5c6210e716a24cd41519b81a54a) )
	ROM_LOAD16_BYTE( "p497.bg2",    0x200001, 0x100000, CRC(5d648914) SHA1(2a4a2839293529aee500aacfbf1d6b12b328b2eb) )
	ROM_LOAD16_BYTE( "p497.bg3",    0x000000, 0x100000, CRC(627e236c) SHA1(a4bd8b482cbac2bf2ab1723ee61d32480ede8985) )
	ROM_LOAD16_BYTE( "p497.bg4",    0x000001, 0x100000, CRC(3698f748) SHA1(856eeed8eff79273ba3aafbbd5d0b1d89e9cff5b) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "betsound.sp1",   0x00000, 0x100000, CRC(979ecd0e) SHA1(827e8c86b27e5252368960fffe42ace167aa4495) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )

	/*** Wheel Controller ***/
	ROM_REGION( 0x100000, "wheelcpu", 0 )
	ROM_LOAD16_BYTE( "wheel.cp1",   0x00001, 0x80000, CRC(57f8a869) SHA1(2a3cdae1bad5e94506b9b42b7f2630c52ffcbbef) )
	ROM_LOAD16_BYTE( "wheel.cp2",   0x00000, 0x80000, CRC(a8441b04) SHA1(cc8f10390947c2a15b2c94b11574c5eeb69fded5) )

	ROM_REGION( 0x800000, "wheelsnd", 0 ) /* the wheel controller has 8 sockets */
	ROM_LOAD( "rwc497ym.sp1",   0x000000, 0x100000, CRC(13d6cff5) SHA1(ad1858f251e11017a427cbf7219d78bb2b854528) )
	ROM_LOAD( "rwc497ym.sp2",   0x100000, 0x100000, CRC(f8c7efd1) SHA1(e86a7ef0617c85415334e1f39a9059d5b16bc7d1) )
	ROM_LOAD( "rwc497ym.sp3",   0x200000, 0x100000, CRC(a1977dff) SHA1(c405bf1f1721ae864a2ff91ec7d637f03e431ad4) )
	ROM_LOAD( "rwc497ym.sp4",   0x300000, 0x100000, CRC(f8cb0fb8) SHA1(3ea8f268bc8745a257eb4b20d7e79196d0f1fb9e) )
	ROM_LOAD( "rwc497ym.sp5",   0x400000, 0x100000, CRC(788b52f7) SHA1(1b339cb984b807a08e6fde260b5ee2bc8ca66f62) )
	ROM_LOAD( "rwc497ym.sp6",   0x500000, 0x100000, CRC(be94fd18) SHA1(2884cae7cf96008a78e77f42e8efb5c3ca8f4a4d) )
ROM_END


/*
   Coinmaster Keno (y2k, spanish, 2000-12-14)
   Physical Unit + 10 bet stations.
*/
ROM_START( cmkenosp )
	/*** Bet Station ***/
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bet1.cp1",   0x00001, 0x80000, CRC(ee04b815) SHA1(cea29973cf9caa5c06bc312fc3b19e146c1ae063) )
	ROM_LOAD16_BYTE( "bet2.cp2",   0x00000, 0x80000, CRC(32071845) SHA1(278217a70ea777f82ae91d11d51b832383eafdbe) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fore1.fg1",   0x00001, 0x80000, CRC(a3548c2a) SHA1(02f98ee09581a235df3704951683f9d2aab3b1e8) )
	ROM_LOAD16_BYTE( "fore2.fg2",   0x00000, 0x80000, CRC(8b1afa73) SHA1(efd176dfb55f047b8e01b9460469936c86953417) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "back1.bg1",   0x200000, 0x100000, CRC(8e9d1753) SHA1(4a733bc6b284571b2dae9e80ba8b88724e9dbffb) )
	ROM_LOAD16_BYTE( "back2.bg2",   0x200001, 0x100000, CRC(aa4fe1ba) SHA1(241cf6ee13664d4cf0c559c26170cb561afca016) )
	ROM_LOAD16_BYTE( "back3.bg3",   0x000000, 0x100000, CRC(9f26f0e0) SHA1(0c3d78e2befc6fdeb8c3534f5278d2d275106219) )
	ROM_LOAD16_BYTE( "back4.bg4",   0x000001, 0x100000, CRC(96d33887) SHA1(ca7eb9f2cfeb65c69e837246c8c78ea56c057e66) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "betsound.sp1",   0x00000, 0x100000, CRC(979ecd0e) SHA1(827e8c86b27e5252368960fffe42ace167aa4495) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )

	/*** Wheel Controller ***/
	ROM_REGION( 0x100000, "wheelcpu", 0 )
	ROM_LOAD16_BYTE( "wheel_prog_1.cp1",   0x00001, 0x80000, CRC(d56b0d2c) SHA1(d8a79fecf8fca92cc4856d9dad9f1d16d51b68a3) )
	ROM_LOAD16_BYTE( "wheel_prog_2.cp2",   0x00000, 0x80000, CRC(cfc02d3e) SHA1(09e41b26c62137b31f8673184dad565932881f47) )

	ROM_REGION( 0x800000, "wheelsnd", 0 ) /* the wheel controller has 8 sockets */
	ROM_LOAD( "rwc497ym.sp1",   0x000000, 0x100000, CRC(90a93951) SHA1(73603f402eb3b62e69a745af9d45738f35bc0b4e) )
	ROM_LOAD( "rwc497ym.sp2",   0x100000, 0x100000, CRC(f5d0a6e7) SHA1(c4a1c333854c95e37c0040fed35b72ac1e853832) )
	ROM_LOAD( "rwc497ym.sp3",   0x200000, 0x100000, CRC(0e53c1a9) SHA1(0785c52b24277c9ba24d0fbf0ac335acb0235e23) )
	ROM_LOAD( "rwc497ym.sp4",   0x300000, 0x100000, CRC(b5729ae7) SHA1(0e63fbb81ff5f2fef3c653f769db8073dff1214b) )
ROM_END


/*
   Coinmaster Keno (y2k, spanish, 2000-12-02)
   Physical Unit + 10 bet stations.
*/
ROM_START( cmkenospa )
	/*** Bet Station ***/
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bet.cp1",   0x00001, 0x80000, CRC(ffdc69a0) SHA1(2ba6a36cb0953474164d4fb80a60bf8ca27e9a0c) )
	ROM_LOAD16_BYTE( "bet.cp2",   0x00000, 0x80000, CRC(c46f237c) SHA1(75a60ace7277a90b3d7acd7838d1271fd41517f1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fore1.fg1",   0x00001, 0x80000, CRC(a3548c2a) SHA1(02f98ee09581a235df3704951683f9d2aab3b1e8) )
	ROM_LOAD16_BYTE( "fore2.fg2",   0x00000, 0x80000, CRC(8b1afa73) SHA1(efd176dfb55f047b8e01b9460469936c86953417) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "back1.bg1",   0x200000, 0x100000, CRC(8e9d1753) SHA1(4a733bc6b284571b2dae9e80ba8b88724e9dbffb) )
	ROM_LOAD16_BYTE( "back2.bg2",   0x200001, 0x100000, CRC(aa4fe1ba) SHA1(241cf6ee13664d4cf0c559c26170cb561afca016) )
	ROM_LOAD16_BYTE( "back3.bg3",   0x000000, 0x100000, CRC(9f26f0e0) SHA1(0c3d78e2befc6fdeb8c3534f5278d2d275106219) )
	ROM_LOAD16_BYTE( "back4.bg4",   0x000001, 0x100000, CRC(96d33887) SHA1(ca7eb9f2cfeb65c69e837246c8c78ea56c057e66) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "betsound.sp1",   0x00000, 0x100000, CRC(979ecd0e) SHA1(827e8c86b27e5252368960fffe42ace167aa4495) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",  0x0000, 0x0200, NO_DUMP )

	/*** Wheel Controller ***/
	ROM_REGION( 0x100000, "wheelcpu", 0 )
	ROM_LOAD16_BYTE( "wheel.cp1",   0x00001, 0x80000, CRC(16f750b7) SHA1(b1c99ec659cda1b2fe6ef9ec39f4cdd8f0ddecec) )
	ROM_LOAD16_BYTE( "wheel.cp2",   0x00000, 0x80000, CRC(49a50ae7) SHA1(89857ebd94ebbfa040d99648a46779c9ba8f85dd) )

	ROM_REGION( 0x800000, "wheelsnd", 0 ) /* the wheel controller has 8 sockets */
	ROM_LOAD( "rwc497ym.sp1",   0x000000, 0x100000, CRC(90a93951) SHA1(73603f402eb3b62e69a745af9d45738f35bc0b4e) )
	ROM_LOAD( "rwc497ym.sp2",   0x100000, 0x100000, CRC(f5d0a6e7) SHA1(c4a1c333854c95e37c0040fed35b72ac1e853832) )
	ROM_LOAD( "rwc497ym.sp3",   0x200000, 0x100000, CRC(0e53c1a9) SHA1(0785c52b24277c9ba24d0fbf0ac335acb0235e23) )
	ROM_LOAD( "rwc497ym.sp4",   0x300000, 0x100000, CRC(b5729ae7) SHA1(0e63fbb81ff5f2fef3c653f769db8073dff1214b) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT        ROT    COMPANY                    FULLNAME                                       FLAGS
GAME( 2000, colorama,  0,        coinmvga, coinmvga, coinmvga_state, empty_init, ROT0,  "Coinmaster-Gaming, Ltd.", "Colorama (P521, English)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, coloramas, colorama, coinmvga, coinmvga, coinmvga_state, empty_init, ROT0,  "Coinmaster-Gaming, Ltd.", "Colorama (P521 V13, Spanish)",                MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, wof_v16,   0,        coinmvga, coinmvga, coinmvga_state, empty_init, ROT0,  "Coinmaster-Gaming, Ltd.", "Wheel of Fortune (P517 V16, English)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, wof_v11,   wof_v16,  coinmvga, coinmvga, coinmvga_state, empty_init, ROT0,  "Coinmaster-Gaming, Ltd.", "Wheel of Fortune (P517 V11, English)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2001, cmrltv75,  0,        coinmvga, coinmvga, coinmvga_state, empty_init, ROT90, "Coinmaster-Gaming, Ltd.", "Coinmaster Roulette P497 V75 (Y2K, Spanish)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, cmkenosp,  0,        coinmvga, coinmvga, coinmvga_state, empty_init, ROT90, "Coinmaster-Gaming, Ltd.", "Coinmaster Keno (Y2K, Spanish, 2000-12-14)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, cmkenospa, cmkenosp, coinmvga, coinmvga, coinmvga_state, empty_init, ROT90, "Coinmaster-Gaming, Ltd.", "Coinmaster Keno (Y2K, Spanish, 2000-12-02)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
