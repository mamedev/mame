// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Atari I, Robot hardware

    Games supported:
        * I, Robot

    Known issues:
        * none at this time

****************************************************************************

    I-Robot Memory Map

    0000 - 07FF  R/W    RAM
    0800 - 0FFF  R/W    Banked RAM
    1000 - 1000  INRD1  Bit 7 = Right Coin
                        Bit 6 = Left Coin
                        Bit 5 = Aux Coin
                        Bit 4 = Self Test
                        Bit 3 = ?
                        Bit 2 = ?
                        Bit 1 = ?
                        Bit 0 = ?
    1040 - 1040  INRD2  Bit 7 = Start 1
                        Bit 6 = Start 2
                        Bit 5 = ?
                        Bit 4 = Fire
                        Bit 3 = ?
                        Bit 2 = ?
                        Bit 1 = ?
                        Bit 0 = ?
    1080 - 1080  STATRD Bit 7 = VBLANK
                        Bit 6 = Polygon generator done
                        Bit 5 = Mathbox done
                        Bit 4 = Unused
                        Bit 3 = ?
                        Bit 2 = ?
                        Bit 1 = ?
                        Bit 0 = ?
    10C0 - 10C0  INRD3  Dip switch
    1140 - 1140  STATWR Bit 7 = Select Polygon RAM banks
                        Bit 6 = BFCALL
                        Bit 5 = Cocktail Flip
                        Bit 4 = Start Mathbox
                        Bit 3 = Connect processor bus to mathbox bus
                        Bit 2 = Start polygon generator
                        Bit 1 = Select polygon image RAM bank
                        Bit 0 = Erase polygon image memory
    1180 - 1180  OUT0   Bit 7 = Alpha Map 1
                        Bit 6,5 = RAM bank select
                        Bit 4,3 = Mathbox memory select
                        Bit 2,1 = Mathbox bank select
    11C0 - 11C0  OUT1   Bit 7 = Coin Counter R
                        Bit 6 = Coin Counter L
                        Bit 5 = LED2
                        Bit 4 = LED1
                        Bit 3,2,1 = ROM bank select
    1200 - 12FF  R/W    NVRAM (bits 0..3 only)
    1300 - 13FF  W      Select analog controller
    1300 - 13FF  R      Read analog controller
    1400 - 143F  R/W    Quad Pokey
    1800 - 18FF         Palette RAM
    1900 - 1900  W      Watchdog reset
    1A00 - 1A00  W      FIREQ Enable
    1B00 - 1BFF  W      Start analog controller ADC
    1C00 - 1FFF  R/W    Character RAM
    2000 - 3FFF  R/W    Mathbox/Vector Gen Shared RAM
    4000 - 5FFF  R      Banked ROM
    6000 - FFFF  R      Fixed ROM

    Notes:
    - There is no flip screen nor cocktail mode in the original game

02/2010 - Added XTAL values based on the parts listing the manual.  The
          divisors for the cpus make sense, however, they are not verified.

****************************************************************************/

#include "emu.h"
#include "irobot.h"
#include "cpu/m6809/m6809.h"
#include "machine/adc0808.h"
#include "machine/nvram.h"
#include "speaker.h"

#define MAIN_CLOCK      12.096_MHz_XTAL
#define VIDEO_CLOCK     20_MHz_XTAL

/*************************************
 *
 *  IRQ acknowledgement
 *
 *************************************/

void irobot_state::irobot_clearirq_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}


void irobot_state::irobot_clearfirq_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


uint8_t irobot_state::quad_pokeyn_r(offs_t offset)
{
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset % 8) | control;

	return m_pokey[pokey_num]->read(pokey_reg);
}

void irobot_state::quad_pokeyn_w(offs_t offset, uint8_t data)
{
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset % 8) | control;

	m_pokey[pokey_num]->write(pokey_reg, data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void irobot_state::irobot_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).bankrw("bank2");
	map(0x1000, 0x103f).portr("IN0");
	map(0x1040, 0x1040).portr("IN1");
	map(0x1080, 0x1080).r(FUNC(irobot_state::irobot_status_r));
	map(0x10c0, 0x10c0).portr("DSW1");
	map(0x1100, 0x1100).w(FUNC(irobot_state::irobot_clearirq_w));
	map(0x1140, 0x1140).w(FUNC(irobot_state::irobot_statwr_w));
	map(0x1180, 0x1180).w(FUNC(irobot_state::irobot_out0_w));
	map(0x11c0, 0x11c0).w(FUNC(irobot_state::irobot_rom_banksel_w));
	map(0x1200, 0x12ff).rw("nvram", FUNC(x2212_device::read), FUNC(x2212_device::write));
	map(0x1300, 0x1300).mirror(0xff).r("adc", FUNC(adc0809_device::data_r));
	map(0x1400, 0x143f).rw(FUNC(irobot_state::quad_pokeyn_r), FUNC(irobot_state::quad_pokeyn_w));
	map(0x1800, 0x18ff).w(FUNC(irobot_state::irobot_paletteram_w));
	map(0x1900, 0x19ff).nopw();            /* Watchdog reset */
	map(0x1a00, 0x1a00).w(FUNC(irobot_state::irobot_clearfirq_w));
	map(0x1b00, 0x1b03).mirror(0xfc).w("adc", FUNC(adc0809_device::address_offset_start_w));
	map(0x1c00, 0x1fff).ram().share("videoram");
	map(0x2000, 0x3fff).rw(FUNC(irobot_state::irobot_sharedmem_r), FUNC(irobot_state::irobot_sharedmem_w));
	map(0x4000, 0x5fff).bankr("bank1");
	map(0x6000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( irobot )
	PORT_START("IN0")   /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")   /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* MB DONE */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* EXT DONE */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("DSW2") /* DSW2 - 5E*/
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) )  PORT_DIPLOCATION("SW5E:1")
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( German ) )
//  Printed Manual States Dip (0x01) adjusts Doodle City playtime:  ON=2M10S / OFF=3M5S
	PORT_DIPNAME( 0x02, 0x02, "Minimum Game Time" )  PORT_DIPLOCATION("SW5E:2")
	PORT_DIPSETTING(    0x00, "90 Seconds on Level 1" )
	PORT_DIPSETTING(    0x02, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW5E:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( None ) )
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW5E:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW5E:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPNAME( 0x80, 0x80, "Demo Mode" )  PORT_DIPLOCATION("SW5E:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") /* DSW1 - 3J */
	PORT_DIPNAME(    0x03, 0x00, "Coins Per Credit" )  PORT_DIPLOCATION("SW3J:!1,!2")
	PORT_DIPSETTING( 0x00, "1 Coin 1 Credit" )
	PORT_DIPSETTING( 0x01, "2 Coins 1 Credit" )
	PORT_DIPSETTING( 0x02, "3 Coins 1 Credit" )
	PORT_DIPSETTING( 0x03, "4 Coins 1 Credit" )
	PORT_DIPNAME(    0x0c, 0x00, "Right Coin" )  PORT_DIPLOCATION("SW3J:!3,!4")
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x04, "1 Coin for 4 Coin Units" )
	PORT_DIPSETTING( 0x08, "1 Coin for 5 Coin Units" )
	PORT_DIPSETTING( 0x0c, "1 Coin for 6 Coin Units" )
	PORT_DIPNAME(    0x10, 0x00, "Left Coin" )  PORT_DIPLOCATION("SW3J:!5")
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x10, "1 Coin for 2 Coin Units" )
	PORT_DIPNAME(    0xe0, 0x00, "Bonus Adder" )  PORT_DIPLOCATION("SW3J:!6,!7,!8")
	PORT_DIPSETTING( 0x00, DEF_STR( None ) )
	PORT_DIPSETTING( 0x20, "1 Credit for 2 Coin Units" )
	PORT_DIPSETTING( 0xa0, "1 Credit for 3 Coin Units" )
	PORT_DIPSETTING( 0x40, "1 Credit for 4 Coin Units" )
	PORT_DIPSETTING( 0x80, "1 Credit for 5 Coin Units" )
	PORT_DIPSETTING( 0x60, "2 Credits for 4 Coin Units" )
	PORT_DIPSETTING( 0xe0, DEF_STR( Free_Play ) )

	PORT_START("AN0")   /* IN4 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(96,163) PORT_SENSITIVITY(70) PORT_KEYDELTA(50)

	PORT_START("AN1")   /* IN5 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(96,159) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_REVERSE
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 4, 5, 6, 7, 12, 13, 14, 15},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16},
	16*8
};


static GFXDECODE_START( gfx_irobot )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 64, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void irobot_state::irobot(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MAIN_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &irobot_state::irobot_map);

	adc0809_device &adc(ADC0809(config, "adc", MAIN_CLOCK / 16));
	adc.in_callback<0>().set_ioport("AN0");
	adc.in_callback<1>().set_ioport("AN1");

	X2212(config, "nvram").set_auto_save(true);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 29*8-1);
	m_screen->set_screen_update(FUNC(irobot_state::screen_update_irobot));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_irobot);
	PALETTE(config, m_palette, FUNC(irobot_state::irobot_palette), 64 + 32); // 64 for polygons, 32 for text

	TIMER(config, "irvg_timer").configure_generic(FUNC(irobot_state::irobot_irvg_done_callback));
	TIMER(config, "irmb_timer").configure_generic(FUNC(irobot_state::irobot_irmb_done_callback));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* FIXME: I-Robot has all channels of the quad-pokey tied together
	 *        This needs to be taken into account in the design.
	 */
	POKEY(config, m_pokey[0], MAIN_CLOCK / 8);
	m_pokey[0]->allpot_r().set_ioport("DSW2");
	m_pokey[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	POKEY(config, m_pokey[1], MAIN_CLOCK / 8);
	m_pokey[1]->add_route(ALL_OUTPUTS, "mono", 0.25);

	POKEY(config, m_pokey[2], MAIN_CLOCK / 8);
	m_pokey[2]->add_route(ALL_OUTPUTS, "mono", 0.25);

	POKEY(config, m_pokey[3], MAIN_CLOCK / 8);
	m_pokey[3]->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( irobot )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code + 48K Banked ROM*/
	ROM_LOAD( "136029-208.bin",     0x06000, 0x2000, CRC(b4d0be59) SHA1(5b476dbee8b171a96301b2204420161333d4ca97) )
	ROM_LOAD( "136029-209.bin",     0x08000, 0x4000, CRC(f6be3cd0) SHA1(a88ae0cc9ee22aa5dd3db0173f24313189f894f8) )
	ROM_LOAD( "136029-210.bin",     0x0c000, 0x4000, CRC(c0eb2133) SHA1(daa77293678b7e822d0672b90789c53098c5451e) )
	ROM_LOAD( "136029-405.bin",     0x10000, 0x4000, CRC(9163efe4) SHA1(5d71d8ec80c9be4726189d48ad519b4638160d64) )
	ROM_LOAD( "136029-206.bin",     0x14000, 0x4000, CRC(e114a526) SHA1(bd94ad4d536f681efa81153050a12098a31d79cf) )
	ROM_LOAD( "136029-207.bin",     0x18000, 0x4000, CRC(b4556cb0) SHA1(2e0c1e4c265e7d232ca86d5c8760e32fc49fe08d) )

	ROM_REGION16_BE( 0x10000, "mathbox", 0 )  /* mathbox region */
	ROM_LOAD16_BYTE( "136029-104.bin", 0x0000,  0x2000, CRC(0a6cdcca) SHA1(b9fd76eae8ca24fa3abc30c46bbf30d89943d97d) )
	ROM_LOAD16_BYTE( "136029-103.bin", 0x0001,  0x2000, CRC(0c83296d) SHA1(c1f4041a58f395e24855254849604dfe3b8b0d71) )  /* ROM data from 0000-bfff */
	ROM_LOAD16_BYTE( "136029-102.bin", 0x4000,  0x4000, CRC(9d588f22) SHA1(787ec3e642e1dc3417477348afa88c764e1f2a88) )
	ROM_LOAD16_BYTE( "136029-101.bin", 0x4001,  0x4000, CRC(62a38c08) SHA1(868bb3fe5657a4ce45c3dd04ba26a7fb5a5ded42) )
	/* RAM data from c000-dfff */
	/* COMRAM from   e000-ffff */

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "136029-124.bin",     0x0000,  0x0800, CRC(848948b6) SHA1(743c6570c787bc9a2a14716adc66b8e2fe57129f) )

	ROM_REGION( 0x3420, "proms", 0 )
	ROM_LOAD( "136029-125.bin",      0x0000,  0x0020, CRC(446335ba) SHA1(5b42cc065bfac467028ae883844c8f94465c3666) )
	ROM_LOAD( "136029-111.bin",      0x0020,  0x0400, CRC(9fbc9bf3) SHA1(33dee2382e1e3899ffbaea859a67af7334270b4a) )    /* program ROMs from c000-f3ff */
	ROM_LOAD( "136029-112.bin",      0x0420,  0x0400, CRC(b2713214) SHA1(4e1ea039e7a3e341796097b0c6943a4805b89f56) )
	ROM_LOAD( "136029-113.bin",      0x0820,  0x0400, CRC(7875930a) SHA1(63a3818450a76d230a75f038b140c3934659313e) )
	ROM_LOAD( "136029-114.bin",      0x0c20,  0x0400, CRC(51d29666) SHA1(34887df0f1ac064b4cf4252a225406e8b30872c6) )
	ROM_LOAD( "136029-115.bin",      0x1020,  0x0400, CRC(00f9b304) SHA1(46b4495002ddf80668a66a4f85cab99432677b50) )
	ROM_LOAD( "136029-116.bin",      0x1420,  0x0400, CRC(326aba54) SHA1(e4caab90910b3aa16c314909f8c02eaf212449a1) )
	ROM_LOAD( "136029-117.bin",      0x1820,  0x0400, CRC(98efe8d0) SHA1(39532fc1b14714396764500a9b1c9e4fed97a970) )
	ROM_LOAD( "136029-118.bin",      0x1c20,  0x0400, CRC(4a6aa7f9) SHA1(163e8e764b400d726c725b6a45901c311e62667e) )
	ROM_LOAD( "136029-119.bin",      0x2020,  0x0400, CRC(a5a13ad8) SHA1(964a87c879c953563ca84f8e3c1201302c7b2b91) )
	ROM_LOAD( "136029-120.bin",      0x2420,  0x0400, CRC(2a083465) SHA1(35ca23d5bbdc2827afb823a974864b96eb135797) )
	ROM_LOAD( "136029-121.bin",      0x2820,  0x0400, CRC(adebcb99) SHA1(4628f8af43d82e578833b1452ec747eeb822b4e4) )
	ROM_LOAD( "136029-122.bin",      0x2c20,  0x0400, CRC(da7b6f79) SHA1(02398ba6e7c56d961bf92e2755e530db1144219d) )
	ROM_LOAD( "136029-123.bin",      0x3020,  0x0400, CRC(39fff18f) SHA1(85f338eeff7d8ed58804611bf8446ebb697d196d) )

	/*  Colorprom from John's driver. ? */
	/*  ROM_LOAD( "136029.125",    0x0000, 0x0020, CRC(c05abf82) ) */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, irobot, 0, irobot, irobot, irobot_state, init_irobot, ROT0, "Atari", "I, Robot", 0 )
