// license:BSD-3-Clause
// copyright-holders:BUT
/*
 *  Thunder Ceptor board
 *  (C) 1986 Namco
 *
 *  Hardware analyzed by nono
 *  Driver by BUT
 */

#include "emu.h"
#include "includes/tceptor.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "sound/ym2151.h"
#include "rendlay.h"
#include "speaker.h"

#include "tceptor2.lh"


/*******************************************************************/

uint8_t tceptor_state::m68k_shared_r(offs_t offset)
{
	return m_m68k_shared_ram[offset];
}

void tceptor_state::m68k_shared_w(offs_t offset, uint8_t data)
{
	m_m68k_shared_ram[offset] = data;
}


/*******************************************************************/

void tceptor_state::m6809_irq_enable_w(uint8_t data)
{
	m_m6809_irq_enable = 1;
}

void tceptor_state::m6809_irq_disable_w(uint8_t data)
{
	m_m6809_irq_enable = 0;
}


void tceptor_state::m68k_irq_enable_w(uint16_t data)
{
	m_m68k_irq_enable = data;
}


void tceptor_state::mcu_irq_enable_w(uint8_t data)
{
	m_mcu_irq_enable = 1;
}

void tceptor_state::mcu_irq_disable_w(uint8_t data)
{
	m_mcu_irq_enable = 0;
}


/* fix dsw/input data to memory mapped data */
uint8_t tceptor_state::fix_input0(uint8_t in1, uint8_t in2)
{
	uint8_t r = 0;

	r |= (in1 & 0x80) >> 7;
	r |= (in1 & 0x20) >> 4;
	r |= (in1 & 0x08) >> 1;
	r |= (in1 & 0x02) << 2;
	r |= (in2 & 0x80) >> 3;
	r |= (in2 & 0x20) >> 0;
	r |= (in2 & 0x08) << 3;
	r |= (in2 & 0x02) << 6;

	return r;
}

uint8_t tceptor_state::fix_input1(uint8_t in1, uint8_t in2)
{
	uint8_t r = 0;

	r |= (in1 & 0x40) >> 6;
	r |= (in1 & 0x10) >> 3;
	r |= (in1 & 0x04) >> 0;
	r |= (in1 & 0x01) << 3;
	r |= (in2 & 0x40) >> 2;
	r |= (in2 & 0x10) << 1;
	r |= (in2 & 0x04) << 4;
	r |= (in2 & 0x01) << 7;

	return r;
}

uint8_t tceptor_state::dsw0_r()
{
	return fix_input0(ioport("DSW1")->read(), ioport("DSW2")->read());
}

uint8_t tceptor_state::dsw1_r()
{
	return fix_input1(ioport("DSW1")->read(), ioport("DSW2")->read());
}

uint8_t tceptor_state::input0_r()
{
	return fix_input0(ioport("BUTTONS")->read(), ioport("SERVICE")->read());
}

uint8_t tceptor_state::input1_r()
{
	return fix_input1(ioport("BUTTONS")->read(), ioport("SERVICE")->read());
}

/*******************************************************************/

void tceptor_state::m6809_map(address_map &map)
{
	map(0x0000, 0x17ff).ram();
	map(0x1800, 0x1bff).ram().w(FUNC(tceptor_state::tceptor_tile_ram_w)).share("tile_ram");
	map(0x1c00, 0x1fff).ram().w(FUNC(tceptor_state::tceptor_tile_attr_w)).share("tile_attr");
	map(0x2000, 0x3fff).ram().w(FUNC(tceptor_state::tceptor_bg_ram_w)).share("bg_ram");  // background (VIEW RAM)
	map(0x4000, 0x43ff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w));
	map(0x4800, 0x4800).w(FUNC(tceptor_state::tceptor2_shutter_w));
	map(0x4f00, 0x4f07).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w));
	map(0x5000, 0x5006).w(FUNC(tceptor_state::tceptor_bg_scroll_w));  // bg scroll
	map(0x6000, 0x7fff).ram().share("m68k_shared_ram"); // COM RAM
	map(0x8000, 0x8000).w(FUNC(tceptor_state::m6809_irq_disable_w));
	map(0x8800, 0x8800).w(FUNC(tceptor_state::m6809_irq_enable_w));
	map(0x8000, 0xffff).rom();
}


void tceptor_state::m6502_a_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("share2");
	map(0x0100, 0x01ff).ram();
	map(0x0200, 0x02ff).ram();
	map(0x0300, 0x030f).ram();
	map(0x2000, 0x2001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x3000, 0x30ff).ram().share("share3");
	map(0x3c01, 0x3c01).nopw();
	map(0x8000, 0xffff).rom();
}


void tceptor_state::m6502_b_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("share2");
	map(0x0100, 0x01ff).ram();
	map(0x4000, 0x4000).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x5000, 0x5000).nopw();           // voice ctrl??
	map(0x8000, 0xffff).rom();
}


void tceptor_state::m68k_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();         // M68K ERROR 1
	map(0x100000, 0x10ffff).rom();         // not sure
	map(0x200000, 0x203fff).ram();         // M68K ERROR 0
	map(0x300000, 0x300001).nopw();
	map(0x400000, 0x4001ff).writeonly().share("sprite_ram");
	map(0x500000, 0x51ffff).w(m_c45_road, FUNC(namco_c45_road_device::write));
	map(0x600000, 0x600001).w(FUNC(tceptor_state::m68k_irq_enable_w));    // not sure
	map(0x700000, 0x703fff).rw(FUNC(tceptor_state::m68k_shared_r), FUNC(tceptor_state::m68k_shared_w)).umask16(0x00ff);
}


void tceptor_state::mcu_map(address_map &map)
{
	map(0x0000, 0x001f).m("mcu", FUNC(hd63701v0_cpu_device::m6801_io));
	map(0x0080, 0x00ff).ram();
	map(0x1000, 0x13ff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w));
	map(0x1400, 0x154d).ram();
	map(0x17c0, 0x17ff).ram();
	map(0x2000, 0x20ff).ram().share("share3");
	map(0x2100, 0x2100).r(FUNC(tceptor_state::dsw0_r));
	map(0x2101, 0x2101).r(FUNC(tceptor_state::dsw1_r));
	map(0x2200, 0x2200).r(FUNC(tceptor_state::input0_r));
	map(0x2201, 0x2201).r(FUNC(tceptor_state::input1_r));
	map(0x8000, 0x8000).w(FUNC(tceptor_state::mcu_irq_disable_w));
	map(0x8800, 0x8800).w(FUNC(tceptor_state::mcu_irq_enable_w));
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xdfff).ram().share("nvram");   // Battery Backup
	map(0xf000, 0xffff).rom();
}



/*******************************************************************/

static INPUT_PORTS_START( tceptor )
	PORT_START("DSW1")  /* DSW 1 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Freeze" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  /* DSW 2 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, "A" )
	PORT_DIPSETTING(    0x03, "B" )
	PORT_DIPSETTING(    0x01, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")   /* Memory Mapped Port */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )    // shot
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )    // bomb
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )    // shot
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )    // bomb
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")   /* Memory Mapped Port */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )     // TEST SW
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PEDAL") /* ADC0809 - 8 CHANNEL ANALOG - CHANNEL 1 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xd6) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_CODE_INC(KEYCODE_Z)

	PORT_START("STICKX")    /* ADC0809 - 8 CHANNEL ANALOG - CHANNEL 2 */
	PORT_BIT(  0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("STICKY")    /* ADC08090 - 8 CHANNEL ANALOG - CHANNEL 3 */
	PORT_BIT(  0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xfe) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)
INPUT_PORTS_END

static INPUT_PORTS_START( tceptor2 )
	PORT_INCLUDE( tceptor )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x04, "Mode" )
	PORT_DIPSETTING(    0x00, "2D" )
	PORT_DIPSETTING(    0x04, "3D" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*******************************************************************/

static const gfx_layout tile_layout =
{
	8, 8,
	512,
	2,
	{ 0x0000, 0x0004 }, //,  0x8000, 0x8004 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0*8+0, 0*8+1, 0*8+2, 0*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	2*8*8
};

static GFXDECODE_START( gfx_tceptor )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout,     0,  256 )

	/* decode in video_start */
	//GFXDECODE_ENTRY( "gfx2", 0, bg_layout,    2048,   64 )
	//GFXDECODE_ENTRY( "gfx3", 0, spr16_layout, 1024,   64 )
	//GFXDECODE_ENTRY( "gfx4", 0, spr32_layout, 1024,   64 )
GFXDECODE_END


/*******************************************************************/

void tceptor_state::machine_start()
{
	save_item(NAME(m_m6809_irq_enable));
	save_item(NAME(m_m68k_irq_enable));
	save_item(NAME(m_mcu_irq_enable));

	m_shutter.resolve();
}


/*******************************************************************/

void tceptor_state::machine_reset()
{
	m_m6809_irq_enable = 0;
	m_m68k_irq_enable = 0;
	m_mcu_irq_enable = 0;
}

/*******************************************************************/

void tceptor_state::tceptor(machine_config &config)
{
	/* basic machine hardware */
	M6809(config, m_maincpu, XTAL(49'152'000)/32);
	m_maincpu->set_addrmap(AS_PROGRAM, &tceptor_state::m6809_map);

	M65C02(config, m_audiocpu[0], XTAL(49'152'000)/24);
	m_audiocpu[0]->set_addrmap(AS_PROGRAM, &tceptor_state::m6502_a_map);

	M65C02(config, m_audiocpu[1], XTAL(49'152'000)/24);
	m_audiocpu[1]->set_addrmap(AS_PROGRAM, &tceptor_state::m6502_b_map);

	M68000(config, m_subcpu, XTAL(49'152'000)/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &tceptor_state::m68k_map);

	HD63701V0(config, m_mcu, XTAL(49'152'000)/8); // or compatible 6808 with extra instructions
	m_mcu->set_addrmap(AS_PROGRAM, &tceptor_state::mcu_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	adc0809_device &adc(ADC0809(config, "adc", 1000000)); // unknown clock (needs to >640khz or the wait loop is too fast)
	adc.in_callback<0>().set_constant(0); // unknown
	adc.in_callback<1>().set_ioport("PEDAL");
	adc.in_callback<2>().set_ioport("STICKX");
	adc.in_callback<3>().set_ioport("STICKY");

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tceptor);
	PALETTE(config, m_palette, FUNC(tceptor_state::tceptor_palette), 4096, 1024);

	NAMCO_C45_ROAD(config, m_c45_road, 0);
	m_c45_road->set_palette(m_palette);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.606060);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(38*8, 32*8);
	m_screen->set_visarea(2*8, 34*8-1 + 2*8, 0*8, 28*8-1 + 0);
	m_screen->set_screen_update(FUNC(tceptor_state::screen_update_tceptor));
	m_screen->screen_vblank().set(FUNC(tceptor_state::screen_vblank_tceptor));
	m_screen->set_palette(m_palette);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, "ymsnd", XTAL(14'318'181)/4).add_route(0, "lspeaker", 1.0).add_route(1, "rspeaker", 1.0);

	NAMCO_CUS30(config, m_cus30, XTAL(49'152'000)/2048);
	m_cus30->set_voices(8);
	m_cus30->set_stereo(true);
	m_cus30->add_route(0, "lspeaker", 0.40);
	m_cus30->add_route(1, "rspeaker", 0.40);

	dac_8bit_r2r_device &dac(DAC_8BIT_R2R(config, "dac", 0)); // unknown DAC
	dac.add_route(ALL_OUTPUTS, "lspeaker", 0.4);
	dac.add_route(ALL_OUTPUTS, "rspeaker", 0.4);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tceptor )
	ROM_REGION( 0x10000, "maincpu", 0 )         // 68A09EP
	ROM_LOAD( "tc1-1.10f",  0x08000, 0x08000, CRC(4c6b063e) SHA1(d9701657186f8051391084f51a720037f9f418b1) )

	ROM_REGION( 0x10000, "audiocpu1", 0 )            // RC65C02
	ROM_LOAD( "tc1-21.1m",  0x08000, 0x08000, CRC(2d0b2fa8) SHA1(16ecd70954e52a8661642b15a5cf1db51783e444) )

	ROM_REGION( 0x10000, "audiocpu2", 0 )          // RC65C02
	ROM_LOAD( "tc1-22.3m",  0x08000, 0x08000, CRC(9f5a3e98) SHA1(2b2ffe39fe647a3039b92721817bddc9e9a92d82) )

	ROM_REGION( 0x110000, "sub", 0 )            // MC68000-12
	ROM_LOAD16_BYTE( "tc1-4.8c",     0x000000, 0x08000, CRC(ae98b673) SHA1(5da1c69dd40db9bad2e3d4dc2af3a949172af940) )
	ROM_LOAD16_BYTE( "tc1-3.10c",    0x000001, 0x08000, CRC(779a4b25) SHA1(8563213a1f1caee0eb88aa4bbd37c6004f16b309) )
	// socket 8d and 10d are emtpy

	ROM_REGION( 0x10000, "mcu", 0 )         // Custom 60A1
	ROM_LOAD( "tc1-2.3a",       0x08000, 0x4000, CRC(b6def610) SHA1(d0eada92a25d0243206fb8239374f5757caaea47) ) /* subprogram for the mcu */
	ROM_LOAD( "cus60-60a1.mcu", 0x0f000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) ) /* mcu internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )    // text tilemap
	ROM_LOAD( "tc1-18.6b",  0x00000, 0x02000, CRC(662b5650) SHA1(ba82fe5efd1011854a6d0d7d87075475b65c0601) )

	ROM_REGION( 0x10000, "gfx2", 0 )    // background
	ROM_LOAD( "tc1-20.10e", 0x00000, 0x08000, CRC(3e5054b7) SHA1(ed359f8659a4a46d5ff7299d0da10550b1496db8) )
	ROM_LOAD( "tc1-19.10d", 0x08000, 0x04000, CRC(7406e6e7) SHA1(61ad77667e94fd7e11037da2721f7bbe0130286a) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // 16x16 sprites
	ROM_LOAD( "tc1-16.8t",  0x00000, 0x08000, CRC(7c72be33) SHA1(397e11727b86688d550c28fbdcb864bb9335d891) )
	ROM_LOAD( "tc1-15.10t", 0x08000, 0x08000, CRC(51268075) SHA1(75b6b935c6721adbc984795b9bf0a791fb8b209e) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // 32x32 sprites
	ROM_LOAD( "tc1-8.8m",   0x00000, 0x10000, CRC(192a1f1f) SHA1(8424a6a19c080da0a83e173e33915f4d9326f379) )
	ROM_LOAD( "tc1-10.8p",  0x10000, 0x08000, CRC(7876bcef) SHA1(09180b26d0eab51de18a13723f46d763541979fb) )
	ROM_RELOAD(             0x18000, 0x08000 )
	ROM_LOAD( "tc1-12.8r",  0x20000, 0x08000, CRC(e8f55842) SHA1(7397c8f279b9ddb7d9daf16f307669257a3fd9df) )
	ROM_RELOAD(             0x28000, 0x08000 )
	ROM_LOAD( "tc1-14.8s",  0x30000, 0x08000, CRC(723acf62) SHA1(fa62ffa2a641629803537d0ef1ad30688b04f9ca) )
	ROM_RELOAD(             0x38000, 0x08000 )
	ROM_LOAD( "tc1-7.10m",  0x40000, 0x10000, CRC(828c80d5) SHA1(6d441cbb333aee21f9c3d9608aec951130f9b0c5) )
	ROM_LOAD( "tc1-9.10p",  0x50000, 0x08000, CRC(145cf59b) SHA1(0639a36030823ccd7a476561a8fe61724c8be9d3) )
	ROM_RELOAD(             0x58000, 0x08000 )
	ROM_LOAD( "tc1-11.10r", 0x60000, 0x08000, CRC(ad7c6c7e) SHA1(2ae889c135c6ee924dc336895f7b9b8a98b715d0) )
	ROM_RELOAD(             0x68000, 0x08000 )
	ROM_LOAD( "tc1-13.10s", 0x70000, 0x08000, CRC(e67cef29) SHA1(ba8559caf498bbc1d9278d74da03ee2d910f76d8) )
	ROM_RELOAD(             0x78000, 0x08000 )

	ROM_REGION( 0x3500, "proms", 0 )
	ROM_LOAD( "tc1-3.1k",   0x00000, 0x00400, CRC(fd2fcb57) SHA1(97d5b7527714acfd729b26ac56f0a9210982c551) )    // red components
	ROM_LOAD( "tc1-1.1h",   0x00400, 0x00400, CRC(0241cf67) SHA1(9b2b579425b72a5b1f2c632f53d1c1d172b4ed1e) )    // green components
	ROM_LOAD( "tc1-2.1j",   0x00800, 0x00400, CRC(ea9eb3da) SHA1(0d7cfceac57afc53a063d7fe67cfc9bda0a8dbc8) )    // blue components
	ROM_LOAD( "tc1-5.6a",   0x00c00, 0x00400, CRC(afa8eda8) SHA1(783efbcbf0bb7e4cf2e2618ddd0ef3b52a4518cc) )    // tiles color table
	ROM_LOAD( "tc1-6.7s",   0x01000, 0x00400, CRC(72707677) SHA1(122c1b619c9efa3b7055908dda3102ee28230504) )    // sprite color table
	ROM_LOAD( "tc1-4.2e",   0x01400, 0x00100, CRC(a4e73d53) SHA1(df8231720e9b57cf2751f86ac3ed7433804f51ca) )    // road color table
	ROM_LOAD( "tc1-17.7k",  0x01500, 0x02000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )    // sprite related
ROM_END

ROM_START( tceptor2 )
	ROM_REGION( 0x10000, "maincpu", 0 )         // 68A09EP
	ROM_LOAD( "tc2-1.10f",  0x08000, 0x08000, CRC(f953f153) SHA1(f4cd0a133d23b4bf3c24c70c28c4ecf8ad4daf6f) )

	ROM_REGION( 0x10000, "audiocpu1", 0 )            // RC65C02
	ROM_LOAD( "tc1-21.1m",  0x08000, 0x08000, CRC(2d0b2fa8) SHA1(16ecd70954e52a8661642b15a5cf1db51783e444) )

	ROM_REGION( 0x10000, "audiocpu2", 0 )          // RC65C02
	ROM_LOAD( "tc1-22.3m",  0x08000, 0x08000, CRC(9f5a3e98) SHA1(2b2ffe39fe647a3039b92721817bddc9e9a92d82) )

	ROM_REGION( 0x110000, "sub", 0 )            // MC68000-12
	ROM_LOAD16_BYTE( "tc2-4.8c",     0x000000, 0x08000, CRC(6c2efc04) SHA1(3a91f5b8bbf7040083e2da2bd0fb2ab3c51ec45c) )
	ROM_LOAD16_BYTE( "tc2-3.10c",    0x000001, 0x08000, CRC(312b781a) SHA1(37bf3ced16b765d78bf8de7a4916c2b518b702ed) )
	ROM_LOAD16_BYTE( "tc2-6.8d",     0x100000, 0x08000, CRC(20711f14) SHA1(39623592bb4be3b3be2bff4b3219ac16ba612761) )
	ROM_LOAD16_BYTE( "tc2-5.10d",    0x100001, 0x08000, CRC(925f2560) SHA1(81fcef6a9c7e9dfb6884043cf2266854bc87cd69) )

	ROM_REGION( 0x10000, "mcu", 0 )         // Custom 60A1
	ROM_LOAD( "tc1-2.3a",       0x08000, 0x4000, CRC(b6def610) SHA1(d0eada92a25d0243206fb8239374f5757caaea47) ) /* subprogram for the mcu */
	ROM_LOAD( "cus60-60a1.mcu", 0x0f000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) ) /* mcu internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )    // text tilemap
	ROM_LOAD( "tc1-18.6b",  0x00000, 0x02000, CRC(662b5650) SHA1(ba82fe5efd1011854a6d0d7d87075475b65c0601) )

	ROM_REGION( 0x10000, "gfx2", 0 )    // background
	ROM_LOAD( "tc2-20.10e", 0x00000, 0x08000, CRC(e72738fc) SHA1(53664400f343acdc1d8cf7e00e261ae42b857a5f) )
	ROM_LOAD( "tc2-19.10d", 0x08000, 0x04000, CRC(9c221e21) SHA1(58bcbb998dcf2190cf46dd3d22b116ac673285a6) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // 16x16 sprites
	ROM_LOAD( "tc2-16.8t",  0x00000, 0x08000, CRC(dcf4da96) SHA1(e953cb46d60171271128b3e0ef4e958d1fab1d04) )
	ROM_LOAD( "tc2-15.10t", 0x08000, 0x08000, CRC(fb0a9f89) SHA1(cc9be6ff542b5d5e6ad3baca7a355b9bd31b3dd1) )

	ROM_REGION( 0x80000, "gfx4", 0 )    // 32x32 sprites
	ROM_LOAD( "tc2-8.8m",   0x00000, 0x10000, CRC(03528d79) SHA1(237810fa55c36b6d87c7e02e02f19feb64e5a11f) )
	ROM_LOAD( "tc2-10.8p",  0x10000, 0x10000, CRC(561105eb) SHA1(101a0e48a740ce4acc34a7d1a50191bb857e7371) )
	ROM_LOAD( "tc2-12.8r",  0x20000, 0x10000, CRC(626ca8fb) SHA1(0b51ced00b3de1f672f6f8c7cc5dd9e2ea2e4f8d) )
	ROM_LOAD( "tc2-14.8s",  0x30000, 0x10000, CRC(b9eec79d) SHA1(ae69033d6f80be0be883f919544c167e8f91db27) )
	ROM_LOAD( "tc2-7.10m",  0x40000, 0x10000, CRC(0e3523e0) SHA1(eb4670333ad383099fafda1c930f42e48e82f5c5) )
	ROM_LOAD( "tc2-9.10p",  0x50000, 0x10000, CRC(ccfd9ff6) SHA1(2934e098aa5231af18dbfb888fe05faab9576a7d) )
	ROM_LOAD( "tc2-11.10r", 0x60000, 0x10000, CRC(40724380) SHA1(57549094fc8403f1f528e57fe3fa64844bf89e22) )
	ROM_LOAD( "tc2-13.10s", 0x70000, 0x10000, CRC(519ec7c1) SHA1(c4abe279d7cf6f626dcbb6f6c4dc2a138b818f51) )

	ROM_REGION( 0x3500, "proms", 0 )
	ROM_LOAD( "tc2-3.1k",   0x00000, 0x00400, CRC(e3504f1a) SHA1(1ac3968e993030a6b2f4719702ff870267ab6918) )    // red components
	ROM_LOAD( "tc2-1.1h",   0x00400, 0x00400, CRC(e8a96fda) SHA1(42e5d2b351000ac0705b01ab484c5fe8e294a08b) )    // green components
	ROM_LOAD( "tc2-2.1j",   0x00800, 0x00400, CRC(c65eda61) SHA1(c316b748daa6be68eebbb480557637efc9f44781) )    // blue components
	ROM_LOAD( "tc1-5.6a",   0x00c00, 0x00400, CRC(afa8eda8) SHA1(783efbcbf0bb7e4cf2e2618ddd0ef3b52a4518cc) )    // tiles color table
	ROM_LOAD( "tc2-6.7s",   0x01000, 0x00400, CRC(badcda76) SHA1(726e0019241d31716f3af9ebe900089bce771477) )    // sprite color table
	ROM_LOAD( "tc2-4.2e",   0x01400, 0x00100, CRC(6b49fc30) SHA1(66ca39cd7985643acd71905111ae2d931c082465) )    // road color table
	ROM_LOAD( "tc1-17.7k",  0x01500, 0x02000, CRC(90db1bf6) SHA1(dbb9e50a8efc3b4012fcf587cc87da9ef42a1b80) )    // sprite related
ROM_END


//     YEAR  NAME      PARENT   MACHINE  INPUT     CLASS          INIT        MONITOR  COMPANY  FULLNAME                 FLAGS )
GAME(  1986, tceptor,  0,       tceptor, tceptor,  tceptor_state, empty_init, ROT0,    "Namco", "Thunder Ceptor",        0)
GAMEL( 1986, tceptor2, tceptor, tceptor, tceptor2, tceptor_state, empty_init, ROT0,    "Namco", "3-D Thunder Ceptor II", 0, layout_tceptor2)
