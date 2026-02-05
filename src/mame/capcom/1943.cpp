// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    1943: The Battle of Midway
    Capcom

    driver by Paul Leaman

    Games supported:
        * 1943: The Battle of Midway (3 regions)
        * 1943: The Battle of Midway Mark II (US)
        * 1943 Kai: Midway Kaisen (Japan)

***************************************************************************/

/*

    SERVICE TEST INFORMATION:

    - To access the unreachable tests (#4-7) in the Service Test, when starting
    up the machine after it has been set into Test Mode, hold down the left
    coin switch.


    TODO:

    - use priority PROM for drawing sprites
    - find and dump an unmodified bme01.12d to correct the 1943 Euro set
    - Accurate interrupt handling

    Notes:

    - The MCU is actually hooked up to a lot more than it actually uses.
      It can potentially communicate with the audio CPU, read the current
      vertical line and has a vblank counter. All this isn't used by the MCU
      program, it only exchanges a value with the main CPU.

*/

#include "emu.h"
#include "1943.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "speaker.h"


/* Interrupts */

TIMER_DEVICE_CALLBACK_MEMBER(_1943_state::scanline)
{
	const int scanline = param;

	// 2 main interrupts per frame (mid-screen interrupt is 96 scanlines before vblank)
	if (scanline == 144 || scanline == 240)
		m_maincpu->set_input_line(0, HOLD_LINE);

	// 4 audio interrupts per frame on 32V
	if ((scanline % 64) == 32)
		m_audiocpu->set_input_line(0, HOLD_LINE);
}


/* Protection Handlers */

void _1943_state::mcu_p3_w(u8 data)
{
	// write strobe
	if (BIT(m_mcu_p3, 6) == 1 && BIT(data, 6) == 0)
	{
		m_mcu_to_cpu = m_mcu_p0;
		m_mcu_to_audiocpu = m_mcu_p2;
	}

	m_mcu_p3 = data;
}


/* Memory Maps */

void _1943_state::c1943_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSWA");
	map(0xc004, 0xc004).portr("DSWB");
	map(0xc007, 0xc007).lr8(NAME([this] () -> u8 { return m_mcu_to_cpu; }));
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc804, 0xc804).w(FUNC(_1943_state::control_w)); // ROM bank switch, screen flip
	map(0xc806, 0xc806).w(m_spriteram, FUNC(buffered_spriteram8_device::write)); // 86S105 DMA transfer request
	map(0xc807, 0xc807).lw8(NAME([this] (u8 data) { m_cpu_to_mcu = data; }));
	map(0xd000, 0xd3ff).ram().w(FUNC(_1943_state::videoram_w)).share("videoram");
	map(0xd400, 0xd7ff).ram().w(FUNC(_1943_state::colorram_w)).share("colorram");
	map(0xd800, 0xd801).ram().share("scrollx");
	map(0xd802, 0xd802).ram().share("scrolly");
	map(0xd803, 0xd804).ram().share("bgscrollx");
	map(0xd806, 0xd806).w(FUNC(_1943_state::layer_w)); // sprites, bg1, bg2 enable
	map(0xd808, 0xd808).nopw(); // ???
	map(0xd868, 0xd868).nopw(); // ???
	map(0xd888, 0xd888).nopw(); // ???
	map(0xd8a8, 0xd8a8).nopw(); // ???
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).ram().share("spriteram");
}

void _1943_state::c1943b_map(address_map &map)
{
	c1943_map(map);

	// the bootleg expects 0x00 to be returned from the protection reads
	// because the protection has been patched out
	map(0xc007, 0xc007).lr8(NAME([]() -> u8 { return 0x00; }));
	map(0xc807, 0xc807).noprw();
}

void _1943_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc800).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xd800, 0xd800).lrw8(NAME([this]() { return m_mcu_to_audiocpu; }), NAME([this](u8 data) { m_audiocpu_to_mcu = data; }));
	map(0xe000, 0xe001).w("ym1", FUNC(ym2203_device::write));
	map(0xe002, 0xe003).w("ym2", FUNC(ym2203_device::write));
}


/* Input Ports */

static INPUT_PORTS_START( 1943 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x08, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SWA:8,7,6,5")
	PORT_DIPSETTING(    0x0f, "1 (Easy)" )
	PORT_DIPSETTING(    0x0e, "2" )
	PORT_DIPSETTING(    0x0d, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x0b, "5" )
	PORT_DIPSETTING(    0x0a, "6" )
	PORT_DIPSETTING(    0x09, "7" )
	PORT_DIPSETTING(    0x08, "8 (Normal)" )
	PORT_DIPSETTING(    0x07, "9" )
	PORT_DIPSETTING(    0x06, "10" )
	PORT_DIPSETTING(    0x05, "11" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x03, "13" )
	PORT_DIPSETTING(    0x02, "14" )
	PORT_DIPSETTING(    0x01, "15" )
	PORT_DIPSETTING(    0x00, "16 (Difficult)" )
	PORT_DIPNAME( 0x10, 0x10, "2 Player Game" )             PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, "1 Credit/2 Players" )
	PORT_DIPSETTING(    0x10, "2 Credits/2 Players" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Screen Stop" )               PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SWA:1" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWB:8,7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SWB:5,4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 2048 characters */
	2,  /* 2 bits per pixel */
	{ 4, 0 },
	{ STEP4(0,1), STEP4(4*2,1) },
	{ STEP8(0,4*2*2) },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	32,32,  /* 32*32 tiles */
	RGN_FRAC(1,2),
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1),        STEP4(4*2,1),          STEP4(4*2*2*32,1), STEP4(4*2*2*32+4*2,1),
	  STEP4(4*2*2*64,1), STEP4(4*2*2*64+4*2,1), STEP4(4*2*2*96,1), STEP4(4*2*2*96+4*2,1) },
	{ STEP32(0,4*2*2) },
	256*8   /* every tile takes 256 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1), STEP4(4*2,1), STEP4(4*2*2*16,1), STEP4(4*2*2*16+4*2,1) },
	{ STEP16(0,4*2*2) },
	64*8    /* every sprite takes 64 consecutive bytes */
};

/* Graphics Decode Info */

static GFXDECODE_START( gfx_1943 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,                  0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,               32*4, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,         32*4+16*16, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout, 32*4+16*16+16*16, 16 )
GFXDECODE_END


/* Machine Driver */

void _1943_state::machine_start()
{
	save_item(NAME(m_cpu_to_mcu));
	save_item(NAME(m_mcu_to_cpu));
	save_item(NAME(m_audiocpu_to_mcu));
	save_item(NAME(m_mcu_to_audiocpu));
	save_item(NAME(m_mcu_p0));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_mcu_p3));
}

void _1943_state::machine_reset()
{
	// these latches are cleared at RESET
	control_w(0);
	layer_w(0);
}

void _1943_state::_1943(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(24'000'000)/4); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &_1943_state::c1943_map);

	Z80(config, m_audiocpu, XTAL(24'000'000)/8); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &_1943_state::sound_map);

	I8751(config, m_mcu, XTAL(24'000'000)/8); /* verified on pcb */
	m_mcu->port_in_cb<0>().set([this](){ return m_cpu_to_mcu; });
	m_mcu->port_out_cb<0>().set([this](u8 data){ m_mcu_p0 = data; });
	m_mcu->port_in_cb<1>().set([this]{ return m_screen->vpos(); });
	m_mcu->port_in_cb<2>().set([this](){ return m_audiocpu_to_mcu; });
	m_mcu->port_out_cb<2>().set([this](u8 data){ m_mcu_p2 = data; });
	m_mcu->port_out_cb<3>().set(FUNC(_1943_state::mcu_p3_w));

	TIMER(config, "scantimer").configure_scanline(FUNC(_1943_state::scanline), "screen", 0, 16);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(24'000'000)/4, 384, 0, 256, 262, 16, 240); // hsync is 306..333 (offset by 128), vsync is 251..253 (offset by 6)
	m_screen->set_screen_update(FUNC(_1943_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_mcu, MCS51_INT1_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_1943);
	PALETTE(config, m_palette, FUNC(_1943_state::_1943_palette), 32*4+16*16+16*16+16*16, 256);

	BUFFERED_SPRITERAM8(config, m_spriteram);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(24'000'000)/16)); /* verified on pcb */
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.10);

	ym2203_device &ym2(YM2203(config, "ym2", XTAL(24'000'000)/16)); /* verified on pcb */
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.10);
}

void _1943_state::_1943b(machine_config &config)
{
	_1943(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &_1943_state::c1943b_map);
	m_screen->screen_vblank().remove();

	config.device_remove("mcu");
}


/* ROMs */

ROM_START( 1943 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bme01.12d", 0x00000, 0x08000, BAD_DUMP CRC(55fd447e) SHA1(f9125745ce85282aa487f744cbf509f335dc3e85) ) /* This rom was hacked, we need a dump of an original */
	ROM_LOAD( "bme02.13d", 0x10000, 0x10000, CRC(073fc57c) SHA1(6824fa387badd3c420f5c9e2b68159ac8a3aaec7) )
	ROM_LOAD( "bme03.14d", 0x20000, 0x10000, CRC(835822c2) SHA1(2c2fad13f062069efa7721abb9d807fb5a7625b4) ) /* These 3 roms have a BLUE stripe on them */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h", 0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )   /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "bm17.12f", 0x10000, 0x8000, CRC(46bcdd07) SHA1(38feda668be25d1adc04aa36afc73b07c1545f89) )
	ROM_LOAD( "bm18.14f", 0x18000, 0x8000, CRC(e6ae7ba0) SHA1(959c306dc28b9be2adc54b3d46312d26764c7b8b) )
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "bm21.12j", 0x30000, 0x8000, CRC(9bfb0d89) SHA1(f1bae7ec46edcf46c7af84c054e89b322f8c8972) )
	ROM_LOAD( "bm22.14j", 0x38000, 0x8000, CRC(04f3c274) SHA1(932780c04abe285e1ec67b726b145175f73eafe0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943u )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bmu01c.12d", 0x00000, 0x08000, CRC(c686cc5c) SHA1(5efb2d9df737564d599f71b71a6438f7624b27c3) )
	ROM_LOAD( "bmu02c.13d", 0x10000, 0x10000, CRC(d8880a41) SHA1(2f9b6a3922efa05eed66c63284bace5f337304ac) )
	ROM_LOAD( "bmu03c.14d", 0x20000, 0x10000, CRC(3f0ee26c) SHA1(8da74fe91a6be3f23fc625f2a433f1f79c424994) ) /* These 3 roms have a RED stripe on them */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h", 0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )   /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "bm17.12f", 0x10000, 0x8000, CRC(46bcdd07) SHA1(38feda668be25d1adc04aa36afc73b07c1545f89) )
	ROM_LOAD( "bm18.14f", 0x18000, 0x8000, CRC(e6ae7ba0) SHA1(959c306dc28b9be2adc54b3d46312d26764c7b8b) )
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "bm21.12j", 0x30000, 0x8000, CRC(9bfb0d89) SHA1(f1bae7ec46edcf46c7af84c054e89b322f8c8972) )
	ROM_LOAD( "bm22.14j", 0x38000, 0x8000, CRC(04f3c274) SHA1(932780c04abe285e1ec67b726b145175f73eafe0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943ua )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bmu01.12d", 0x00000, 0x08000, CRC(793cf15f) SHA1(7d49fd17acad7900c3d1187d25ada14247e267f3) ) /* no revision designation on these */
	ROM_LOAD( "bmu02.13d", 0x10000, 0x10000, CRC(6f1353d5) SHA1(4b264f326891187f93d9fc347194091263828296) )
	ROM_LOAD( "bmu03.14d", 0x20000, 0x10000, CRC(9e7c07f7) SHA1(c437e5fd0e8eb87ef50a6d71a7a43efc38e4f128) ) /* These 3 roms have a RED stripe on them */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h", 0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )   /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "bm17.12f", 0x10000, 0x8000, CRC(46bcdd07) SHA1(38feda668be25d1adc04aa36afc73b07c1545f89) )
	ROM_LOAD( "bm18.14f", 0x18000, 0x8000, CRC(e6ae7ba0) SHA1(959c306dc28b9be2adc54b3d46312d26764c7b8b) )
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "bm21.12j", 0x30000, 0x8000, CRC(9bfb0d89) SHA1(f1bae7ec46edcf46c7af84c054e89b322f8c8972) )
	ROM_LOAD( "bm22.14j", 0x38000, 0x8000, CRC(04f3c274) SHA1(932780c04abe285e1ec67b726b145175f73eafe0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943j )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bm01b.12d", 0x00000, 0x08000, CRC(363f9f3d) SHA1(06fbdf1fa2304a000bcb0a151b2ef4be8b291b0b) ) /* "B" stamped in red ink */
	ROM_LOAD( "bm02b.13d", 0x10000, 0x10000, CRC(7f0d7edc) SHA1(ef4aa067a1f55fead821097af215711be0a87075) )
	ROM_LOAD( "bm03b.14d", 0x20000, 0x10000, CRC(7093da2a) SHA1(27456593e47d69506a27804e32088b15c1f7d767) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h", 0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )   /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "bm17.12f", 0x10000, 0x8000, CRC(46bcdd07) SHA1(38feda668be25d1adc04aa36afc73b07c1545f89) )
	ROM_LOAD( "bm18.14f", 0x18000, 0x8000, CRC(e6ae7ba0) SHA1(959c306dc28b9be2adc54b3d46312d26764c7b8b) )
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "bm21.12j", 0x30000, 0x8000, CRC(9bfb0d89) SHA1(f1bae7ec46edcf46c7af84c054e89b322f8c8972) )
	ROM_LOAD( "bm22.14j", 0x38000, 0x8000, CRC(04f3c274) SHA1(932780c04abe285e1ec67b726b145175f73eafe0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943ja )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bm01.12d", 0x00000, 0x08000, CRC(232df705) SHA1(04d97853d5edef8a6126c81febadbdeda373df29) ) /* Rom labels/names + revision needs to be verified */
	ROM_LOAD( "bm02.13d", 0x10000, 0x10000, CRC(af971575) SHA1(af1d8ce73e8671b7b41248ce6486c9b5aaf6a233) )
	ROM_LOAD( "bm03.14d", 0x20000, 0x10000, CRC(300ec713) SHA1(f66d2356b413a418c887b4085a5315475c7a8bba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h", 0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )   /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "bm17.12f", 0x10000, 0x8000, CRC(46bcdd07) SHA1(38feda668be25d1adc04aa36afc73b07c1545f89) )
	ROM_LOAD( "bm18.14f", 0x18000, 0x8000, CRC(e6ae7ba0) SHA1(959c306dc28b9be2adc54b3d46312d26764c7b8b) )
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "bm21.12j", 0x30000, 0x8000, CRC(9bfb0d89) SHA1(f1bae7ec46edcf46c7af84c054e89b322f8c8972) )
	ROM_LOAD( "bm22.14j", 0x38000, 0x8000, CRC(04f3c274) SHA1(932780c04abe285e1ec67b726b145175f73eafe0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943jah )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bm01_hack.12d", 0x00000, 0x08000, CRC(f6935937) SHA1(6fe8885d734447c2a667cf80dd545200aad6c767) ) /* Rom labels/names + revision needs to be verified */
	ROM_LOAD( "bm02.13d", 0x10000, 0x10000, CRC(af971575) SHA1(af1d8ce73e8671b7b41248ce6486c9b5aaf6a233) )
	ROM_LOAD( "bm03.14d", 0x20000, 0x10000, CRC(300ec713) SHA1(f66d2356b413a418c887b4085a5315475c7a8bba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h", 0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )   /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "bm17.12f", 0x10000, 0x8000, CRC(46bcdd07) SHA1(38feda668be25d1adc04aa36afc73b07c1545f89) )
	ROM_LOAD( "bm18.14f", 0x18000, 0x8000, CRC(e6ae7ba0) SHA1(959c306dc28b9be2adc54b3d46312d26764c7b8b) )
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "bm21.12j", 0x30000, 0x8000, CRC(9bfb0d89) SHA1(f1bae7ec46edcf46c7af84c054e89b322f8c8972) )
	ROM_LOAD( "bm22.14j", 0x38000, 0x8000, CRC(04f3c274) SHA1(932780c04abe285e1ec67b726b145175f73eafe0) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943kai )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "bmk01.12d", 0x00000, 0x08000, CRC(7d2211db) SHA1(b02a0b3daf7e1e224b7cad8fbe93439bd5ec9f0b) )
	ROM_LOAD( "bmk02.13d", 0x10000, 0x10000, CRC(2ebbc8c5) SHA1(3be5ad061411642723e3f2bcb7b3c3caa11ee15f) )
	ROM_LOAD( "bmk03.14d", 0x20000, 0x10000, CRC(475a6ac5) SHA1(fa07a855ba9173b6f81641c806ec7d938b0c282e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bmk05.4k", 0x00000, 0x8000, CRC(25f37957) SHA1(1e50c2a920eb3b5c881843686db857e9fee5ba1d) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bmk04.5h", 0x00000, 0x8000, CRC(884a8692) SHA1(027aa8c868dc07ccd9e27705031107881aef4b91) )   /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* Yes, BM15 & BM19 are NOT BMK */
	ROM_LOAD( "bm15.10f",  0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )  /* bg tiles */
	ROM_LOAD( "bmk16.11f", 0x08000, 0x8000, CRC(9416fe0d) SHA1(92fbc8fffa4497747ab80abe20eef361f6525114) )
	ROM_LOAD( "bmk17.12f", 0x10000, 0x8000, CRC(3d5acab9) SHA1(887d45b648fda952ae2137579f383ab8ede1facd) )
	ROM_LOAD( "bmk18.14f", 0x18000, 0x8000, CRC(7b62da1d) SHA1(1926109a2ab2f550ca87b0d2af73abd2b4a7498d) )
	ROM_LOAD( "bm19.10j",  0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bmk20.11j", 0x28000, 0x8000, CRC(b90364c1) SHA1(104bc02237eeead84c7f35462186d0a1af8761bc) )
	ROM_LOAD( "bmk21.12j", 0x30000, 0x8000, CRC(8c7fe74a) SHA1(8846b57d7f47c10ab1f505c359ecf36dcbacb011) )
	ROM_LOAD( "bmk22.14j", 0x38000, 0x8000, CRC(d5ef8a0e) SHA1(2e42b1fbbfe823a33740a56d1334657db56d24d2) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "bmk24.14k", 0x00000, 0x8000, CRC(bf186ef2) SHA1(cacbb8a61f8a64c3ba4ffde5ca6f07fe120b9a7e) )  /* fg tiles */
	ROM_LOAD( "bmk25.14l", 0x08000, 0x8000, CRC(a755faf1) SHA1(8ee286d6ad7454ae34971f5891ddba4b76c244b0) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bmk06.10a", 0x00000, 0x8000, CRC(5f7e38b3) SHA1(33f69ebe91a0ee45d9107171fed26da475aaab3a) )  /* sprites */
	ROM_LOAD( "bmk07.11a", 0x08000, 0x8000, CRC(ff3751fd) SHA1(bc942ddd46e7b147115e8ac22d24c2d018a7c373) )
	ROM_LOAD( "bmk08.12a", 0x10000, 0x8000, CRC(159d51bd) SHA1(746aa49b18aff0eaf2fb875c573d455416d45a1d) )
	ROM_LOAD( "bmk09.14a", 0x18000, 0x8000, CRC(8683e3d2) SHA1(591dc4811b226fe11cd5441ecb51aa3e95e68ac5) )
	ROM_LOAD( "bmk10.10c", 0x20000, 0x8000, CRC(1e0d9571) SHA1(44ea9603020e9ab717e3e506f7ecf288506c0502) )
	ROM_LOAD( "bmk11.11c", 0x28000, 0x8000, CRC(f1fc5ee1) SHA1(4ffc8e57734d3b59df695b86070511f1c447b992) )
	ROM_LOAD( "bmk12.12c", 0x30000, 0x8000, CRC(0f50c001) SHA1(0e6367d3f0ba39a00ee0fa6e42ae9d43d12da23d) )
	ROM_LOAD( "bmk13.14c", 0x38000, 0x8000, CRC(fd1acf8e) SHA1(88477ff1e5fbbca251d8cd4f241b42618ba64a80) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bmk14.5f", 0x0000, 0x8000, CRC(cf0f5a53) SHA1(dc50f3f937f52910dbd0cedbc232acfed0aa6a42) )    /* front background */
	ROM_LOAD( "bmk23.8k", 0x8000, 0x8000, CRC(17f77ef9) SHA1(8ebb4b440042436ec2db52bad808cced832db77c) )    /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bmk1.12a",  0x0000, 0x0100, CRC(e001ea33) SHA1(4204bdf87820ac84bab2a1b5571a2ee28c4cdfc5) )   /* red component */
	ROM_LOAD( "bmk2.13a",  0x0100, 0x0100, CRC(af34d91a) SHA1(94bc6514c980fdd1cb013ff0819d6f32464c581c) )   /* green component */
	ROM_LOAD( "bmk3.14a",  0x0200, 0x0100, CRC(43e9f6ef) SHA1(e1f58368fe0bd9b53f6c286ce5009b218a5197dc) )   /* blue component */
	ROM_LOAD( "bmk5.7f",   0x0300, 0x0100, CRC(41878934) SHA1(8f28210ab1d409c89600169a136b74a706001cdf) )   /* char lookup table */
	ROM_LOAD( "bmk10.7l",  0x0400, 0x0100, CRC(de44b748) SHA1(0694fb19d98ccda728424436fc7350da7b5bd05e) )   /* foreground lookup table */
	ROM_LOAD( "bmk9.6l",   0x0500, 0x0100, CRC(59ea57c0) SHA1(f961c7e9981cc819c2adf4efdc977841d284a3a2) )   /* foreground palette bank */
	ROM_LOAD( "bmk12.12m", 0x0600, 0x0100, CRC(8765f8b0) SHA1(f32bab8e3587434b864fe97da9423f2335ccba2e) )   /* background lookup table */
	ROM_LOAD( "bmk11.12l", 0x0700, 0x0100, CRC(87a8854e) SHA1(0cbc601b736d566d625867d65e0f7b2abb535c65) )   /* background palette bank */
	ROM_LOAD( "bmk8.8c",   0x0800, 0x0100, CRC(dad17e2d) SHA1(fdb18ddc7574153bb7e27ba08b04b9dc87061c02) )   /* sprite lookup table */
	ROM_LOAD( "bmk7.7c",   0x0900, 0x0100, CRC(76307f8d) SHA1(8d655e2a5c50541795316d924b2f18b55f4b9571) )   /* sprite palette bank */
	ROM_LOAD( "bm4.12c",   0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )   /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",    0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )   /* video timing (not used) */
ROM_END

ROM_START( 1943mii ) /* Prototype, location test or limited release? - PCB had genuine CAPCOM labels on the roms */
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "01.12d", 0x00000, 0x08000, CRC(8ba22485) SHA1(ed67992d2cf7dcba72bc9525fbce6d2cb03d78c4) ) /* these 3 had USA hand written in pencil on labels */
	ROM_LOAD( "02.13d", 0x10000, 0x10000, CRC(659a5455) SHA1(c4a2cea51c1326f7e60e404ae4d66e567abc4c96) )
	ROM_LOAD( "03.14d", 0x20000, 0x10000, CRC(159ea771) SHA1(d95ff1773cdc566203befd84e1ba961a7dc8f69b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "05.4k", 0x00000, 0x8000, CRC(25f37957) SHA1(1e50c2a920eb3b5c881843686db857e9fee5ba1d) )

	ROM_REGION( 0x1000, "mcu", 0 ) /*  C8751H-88 MCU Code */
	ROM_LOAD( "bm.7k", 0x0000, 0x1000 , CRC(cf4781bf) SHA1(4d63da5bf39a892499c02a79c7daf33d3a94234a) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "04.5h", 0x00000, 0x8000, CRC(8190e092) SHA1(17ca0fa8e61cc6f478d4807262a0333fdb3e4f94) )   /* characters - had USA hand written in pencil on label */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* Mixture of Midway and Kai roms */
	ROM_LOAD( "bm15.10f", 0x00000, 0x8000, CRC(6b1a0443) SHA1(32337c840ccd6815fd5844c194365c58d708f6dc) )  /* bg tiles */
	ROM_LOAD( "bm16.11f", 0x08000, 0x8000, CRC(23c908c2) SHA1(42b83ff5781be9181802a21ff1b23c17ab1bc5a2) )
	ROM_LOAD( "17.12f",   0x10000, 0x8000, CRC(3d5acab9) SHA1(887d45b648fda952ae2137579f383ab8ede1facd) ) /* Yes it's actually 17 & 18 instead of BMK17 & BMK18 */
	ROM_LOAD( "18.14f",   0x18000, 0x8000, CRC(7b62da1d) SHA1(1926109a2ab2f550ca87b0d2af73abd2b4a7498d) ) /* additional hand written sticker "2-4 YZ SCR" */
	ROM_LOAD( "bm19.10j", 0x20000, 0x8000, CRC(868ababc) SHA1(1c7be905f53c63bad25fbbd9b3cf82d2c7749bc3) )
	ROM_LOAD( "bm20.11j", 0x28000, 0x8000, CRC(0917e5d4) SHA1(62dd277bc1fa54cfe168ae2380bc147bd17f4205) )
	ROM_LOAD( "21.12j",   0x30000, 0x8000, CRC(8c7fe74a) SHA1(8846b57d7f47c10ab1f505c359ecf36dcbacb011) ) /* Yes it's actually 21 & 22 instead of BMK21 & BMK22 */
	ROM_LOAD( "22.14j",   0x38000, 0x8000, CRC(d5ef8a0e) SHA1(2e42b1fbbfe823a33740a56d1334657db56d24d2) ) /* additional hand written sticker "2-4 WX SCR" */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "24.14k", 0x00000, 0x8000, CRC(a0074c68) SHA1(c219de2253d1964ae3e3daf60c5f9a563b94b4eb) )  /* fg tiles */
	ROM_LOAD( "25.14l", 0x08000, 0x8000, CRC(f979b2f2) SHA1(06db7b812cf51b3e4476a56bca410ba04e55b925) )

	ROM_REGION( 0x40000, "gfx4", 0 ) /* Only 08 & 12 match known roms, the rest are unique to this set */
	ROM_LOAD( "06.10a", 0x00000, 0x8000, CRC(b261d5d7) SHA1(4f249c213d2853b8a524baba148730fd4dd1536f) )  /* sprites */
	ROM_LOAD( "07.11a", 0x08000, 0x8000, CRC(2af8a6f2) SHA1(f97a08dbdb57de01c21821ddcc30ebe2d57edb17) )
	ROM_LOAD( "08.12a", 0x10000, 0x8000, CRC(159d51bd) SHA1(746aa49b18aff0eaf2fb875c573d455416d45a1d) )
	ROM_LOAD( "09.14a", 0x18000, 0x8000, CRC(70d9f9a7) SHA1(c8d1d3ab4d8baca7fbb5b1d9b3de72c46af5bbd7) )
	ROM_LOAD( "10.10c", 0x20000, 0x8000, CRC(de539920) SHA1(957ab527032e19e57ab1afa5e5e08763104d4c9a) )
	ROM_LOAD( "11.11c", 0x28000, 0x8000, CRC(a6abf183) SHA1(97cf3d00d23e062e15bcba7914e184b249f2c714) )
	ROM_LOAD( "12.12c", 0x30000, 0x8000, CRC(0f50c001) SHA1(0e6367d3f0ba39a00ee0fa6e42ae9d43d12da23d) )
	ROM_LOAD( "13.14c", 0x38000, 0x8000, CRC(f065f619) SHA1(d45b3a7ce306b3dc7b2ccea2484c13c1ff08a0f7) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "14.5f", 0x0000, 0x8000, CRC(02a899f1) SHA1(0f094d925a6e38e922eb487af80da9c9ee7613aa) )    /* front background */
	ROM_LOAD( "23.8k", 0x8000, 0x8000, CRC(b6dfdf85) SHA1(c223ae136f67e5f9910cbfa49b9827e5122e018e) )    /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
//  PCB had standard BM0x for bproms 1 through 3, but clearly these should use the Kai BPROMs for correct colors
//  BPROMs 4 through 8 macth the Kai set - labels were a non descript yellow dot with prom number
//  BPROMs 9 through 12 are unique - labels were a non descript yellow dot with prom number
	ROM_LOAD( "bmk1.12a", 0x0000, 0x0100, CRC(e001ea33) SHA1(4204bdf87820ac84bab2a1b5571a2ee28c4cdfc5) )   /* red component */
	ROM_LOAD( "bmk2.13a", 0x0100, 0x0100, CRC(af34d91a) SHA1(94bc6514c980fdd1cb013ff0819d6f32464c581c) )   /* green component */
	ROM_LOAD( "bmk3.14a", 0x0200, 0x0100, CRC(43e9f6ef) SHA1(e1f58368fe0bd9b53f6c286ce5009b218a5197dc) )   /* blue component */
	ROM_LOAD( "5.7f",     0x0300, 0x0100, CRC(41878934) SHA1(8f28210ab1d409c89600169a136b74a706001cdf) )   /* char lookup table */
	ROM_LOAD( "10.7l",    0x0400, 0x0100, CRC(db53adf0) SHA1(e3e3a3c262acc628541afa512cfa4ed0c6fc547f) )   /* foreground lookup table */
	ROM_LOAD( "9.6l",     0x0500, 0x0100, CRC(75d5cc90) SHA1(2f04236e7635583fe096c11165fa0a8a0e121d70) )   /* foreground palette bank */
	ROM_LOAD( "12.12m",   0x0600, 0x0100, CRC(784bdf33) SHA1(6a46c2048637770acd3f3d791e1b831e8caf8c99) )   /* background lookup table */
	ROM_LOAD( "11.12l",   0x0700, 0x0100, CRC(6fb2e170) SHA1(91a84f7138c373da0b50d4833de36f17db9a553e) )   /* background palette bank */
	ROM_LOAD( "8.8c",     0x0800, 0x0100, CRC(dad17e2d) SHA1(fdb18ddc7574153bb7e27ba08b04b9dc87061c02) )   /* sprite lookup table */
	ROM_LOAD( "7.7c",     0x0900, 0x0100, CRC(76307f8d) SHA1(8d655e2a5c50541795316d924b2f18b55f4b9571) )   /* sprite palette bank */
	ROM_LOAD( "4.12c",    0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )   /* priority encoder / palette selector (not used) */
	ROM_LOAD( "6.4b",     0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )   /* video timing (not used) */
ROM_END

ROM_START( 1943b )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "1.12d",    0x00000, 0x08000, CRC(9a2d70ab) SHA1(6f84e906656f132ffcb63022f6d067580d261431) ) // protection patched out, disclaimer patched out
	ROM_LOAD( "bm02.13d", 0x10000, 0x10000, CRC(af971575) SHA1(af1d8ce73e8671b7b41248ce6486c9b5aaf6a233) )
	ROM_LOAD( "bm03.14d", 0x20000, 0x10000, CRC(300ec713) SHA1(f66d2356b413a418c887b4085a5315475c7a8bba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k", 0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x8000, "gfx1", 0 ) // logo replaced
	ROM_LOAD( "4.5h", 0x00000, 0x8000, CRC(0aba2096) SHA1(4833ad9f747b529ce92c4993388ab3516f8df4ed) )   /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) // same data, double size roms
	ROM_LOAD( "15.12f", 0x00000, 0x10000, CRC(622b4fba) SHA1(181d7b0a1ca0cfcc3bd71f0b97dc80f7ff27a9c6) )    /* bg tiles */
	ROM_LOAD( "16.14f", 0x10000, 0x10000, CRC(25471a8d) SHA1(a3930dea9b64e84ade78dae4a631ebb7e9741954) )
	ROM_LOAD( "17.12j", 0x20000, 0x10000, CRC(9da79653) SHA1(b9852c476110db3f654152ca85265d184a1a816e) )
	ROM_LOAD( "18.14j", 0x30000, 0x10000, CRC(1f3aced8) SHA1(14ae016279628732b397db9a526bfda7ede0be5a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // same
	ROM_LOAD( "bm24.14k", 0x00000, 0x8000, CRC(11134036) SHA1(88da112ab9fc7e0d8f0e901f273715b950ae588c) )   /* fg tiles */
	ROM_LOAD( "bm25.14l", 0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 ) // same
	ROM_LOAD( "bm06.10a", 0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a", 0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a", 0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a", 0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c", 0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c", 0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c", 0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c", 0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	ROM_LOAD( "bm14.5f", 0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) ) /* front background */
	ROM_LOAD( "bm23.8k", 0x8000, 0x8000, CRC(a52aecbd) SHA1(45b0283d84d394c16c35802463ca95d70d1062d4) ) /* back background */

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",  0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",  0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",  0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",   0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",  0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",   0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m", 0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l", 0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",   0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",   0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",  0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",   0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

ROM_START( 1943bj )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for the banked ROMs images */
	ROM_LOAD( "mkb03.12d", 0x00000, 0x08000, CRC(b3b7c7cd) SHA1(6197023f4384fd2ac72b686c26a6ff2877345b61) ) // protection patched out
	ROM_LOAD( "bm02.13d",  0x10000, 0x10000, CRC(af971575) SHA1(af1d8ce73e8671b7b41248ce6486c9b5aaf6a233) )
	ROM_LOAD( "bm03.14d",  0x20000, 0x10000, CRC(300ec713) SHA1(f66d2356b413a418c887b4085a5315475c7a8bba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bm05.4k",   0x00000, 0x8000, CRC(ee2bd2d7) SHA1(4d2d019a9f8452fbbb247e893280568a2e86073e) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "bm04.5h",   0x00000, 0x8000, CRC(46cb9d3d) SHA1(96fd0e714b91fe13a2ca0d185ada9e4b4baa0c0b) )    /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	/* double size roms - same gfx different layout */
	ROM_LOAD( "mkb12.12f", 0x00000, 0x8000, CRC(075e9a7f) SHA1(2b826d6d202d37cff1aeb58f225e70be7ba9a206) )    /* bg tiles */
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "mkb11.14f", 0x08000, 0x8000, CRC(05aca09a) SHA1(21cc251e61343be27e969885b013fa6e8b5aa210) )
	ROM_CONTINUE( 0x18000, 0x08000 )
	ROM_LOAD( "mkb09.12j", 0x20000, 0x8000, CRC(0f4b7e0e) SHA1(1dd2a4a78ab45bb122895e5a0961e527f77713d1) )
	ROM_CONTINUE( 0x30000, 0x08000 )
	ROM_LOAD( "mkb10.14j", 0x28000, 0x8000, CRC(61a90c0a) SHA1(8aae483e51d645d9e4d2604dbca432c13a3e8d0d) )
	ROM_CONTINUE( 0x38000, 0x08000 )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "mkb08.14k", 0x00000, 0x8000, CRC(798215e3) SHA1(1c732b60cd430aa0acd1698b4fe1984385223b28) )   /* fg tiles */
	ROM_LOAD( "bm25.14l",  0x08000, 0x8000, CRC(092cf9c1) SHA1(19fe3c714b1d52cbb21dea25cdee5af841f525db) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "bm06.10a",  0x00000, 0x8000, CRC(97acc8af) SHA1(c9fa07cb61f6905408b355edabfe453fb652ff0d) )   /* sprites */
	ROM_LOAD( "bm07.11a",  0x08000, 0x8000, CRC(d78f7197) SHA1(6367c7e80e80d4a0d33d7840b5c843c63c80123e) )
	ROM_LOAD( "bm08.12a",  0x10000, 0x8000, CRC(1a626608) SHA1(755c27a07728fd686168e9d9e4dee3d8f274892a) )
	ROM_LOAD( "bm09.14a",  0x18000, 0x8000, CRC(92408400) SHA1(3ab299bad1ba115efead53ebd92254abe7a092ba) )
	ROM_LOAD( "bm10.10c",  0x20000, 0x8000, CRC(8438a44a) SHA1(873629b00cf3f6d8976a7fdafe63cd16e47b7491) )
	ROM_LOAD( "bm11.11c",  0x28000, 0x8000, CRC(6c69351d) SHA1(c213d5c3e76a5749bc32539604716dcef6dcb694) )
	ROM_LOAD( "bm12.12c",  0x30000, 0x8000, CRC(5e7efdb7) SHA1(fef271a38dc1a9e45a0c6e27e28e713c77c8f8c9) )
	ROM_LOAD( "bm13.14c",  0x38000, 0x8000, CRC(1143829a) SHA1(2b3a65e354a205c05a87f783e9938b64bc62396f) )

	ROM_REGION( 0x10000, "tilerom", 0 )    /* tilemaps */
	/* front background */
	ROM_LOAD( "bm14.5f",   0x0000, 0x8000, CRC(4d3c6401) SHA1(ce4f6dbf8fa030ad45cbb5afd58df27fed2d4618) )
	/* back background probably same gfx different layout */
	ROM_LOAD( "mkb07.8k",  0xc000, 0x4000, CRC(ae1b317f) SHA1(d311c198d77ec932d776427e2ebfffe90e5330c3) )
	ROM_CONTINUE( 0x8000, 0x4000 )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "bm1.12a",   0x0000, 0x0100, CRC(74421f18) SHA1(5b8b59f6f4e5ad358611de50608f47f41a5b0e51) )    /* red component */
	ROM_LOAD( "bm2.13a",   0x0100, 0x0100, CRC(ac27541f) SHA1(1796c4c9041dfe28e6319576f21df1dbcb8d12bf) )    /* green component */
	ROM_LOAD( "bm3.14a",   0x0200, 0x0100, CRC(251fb6ff) SHA1(d1118159b3d429d841e4efa938728ebedadd7ec5) )    /* blue component */
	ROM_LOAD( "bm5.7f",    0x0300, 0x0100, CRC(206713d0) SHA1(fa609f6d675af18c379838583505724d28bcff0e) )    /* char lookup table */
	ROM_LOAD( "bm10.7l",   0x0400, 0x0100, CRC(33c2491c) SHA1(13da924e4b182759c4aae49034f3a7cbe556ea65) )    /* foreground lookup table */
	ROM_LOAD( "bm9.6l",    0x0500, 0x0100, CRC(aeea4af7) SHA1(98f4570ee061e9aa58d8ed2d2f8ae59ce2ec5795) )    /* foreground palette bank */
	ROM_LOAD( "bm12.12m",  0x0600, 0x0100, CRC(c18aa136) SHA1(684f04d9a5b94ae1db5fb95763e65271f4cf8e01) )    /* background lookup table */
	ROM_LOAD( "bm11.12l",  0x0700, 0x0100, CRC(405aae37) SHA1(94a06f81b775c4e49d57d42fc064d3072a253bbd) )    /* background palette bank */
	ROM_LOAD( "bm8.8c",    0x0800, 0x0100, CRC(c2010a9e) SHA1(be9852500209066e2f0ff2770e0c217d1636a0b5) )    /* sprite lookup table */
	ROM_LOAD( "bm7.7c",    0x0900, 0x0100, CRC(b56f30c3) SHA1(9f5e6db464d21457a33ec8bdfdff069632b791db) )    /* sprite palette bank */
	ROM_LOAD( "bm4.12c",   0x0a00, 0x0100, CRC(91a8a2e1) SHA1(9583c87eff876f04bc2ccf7218cd8081f1bcdb94) )    /* priority encoder / palette selector (not used) */
	ROM_LOAD( "bm6.4b",    0x0b00, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    /* video timing (not used) */
ROM_END

void _1943_state::init_1943()
{
	u8 *ROM = memregion("maincpu")->base();
	m_mainbank->configure_entries(0, 8, &ROM[0x10000], 0x4000);
}


/* Game Drivers */

//    YEAR  NAME     PARENT  MACHINE  INPUT  CLASS        INIT       ROT     COMPANY    FULLNAME, FLAGS
GAME( 1987, 1943,    0,      _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: The Battle of Midway (Euro)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943u,   1943,   _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: The Battle of Midway (US, Rev C)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943ua,  1943,   _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: The Battle of Midway (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943j,   1943,   _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: Midway Kaisen (Japan, Rev B)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943ja,  1943,   _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: Midway Kaisen (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943jah, 1943,   _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: Midway Kaisen (Japan, no protection hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943b,   1943,   _1943b,  1943,  _1943_state, init_1943, ROT270, "bootleg", "1943: Battle of Midway (bootleg, hack of Japan set)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943bj,  1943,   _1943b,  1943,  _1943_state, init_1943, ROT270, "bootleg", "1943: Midway Kaisen (bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, 1943kai, 0,      _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943 Kai: Midway Kaisen (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, 1943mii, 0,      _1943,   1943,  _1943_state, init_1943, ROT270, "Capcom",  "1943: The Battle of Midway Mark II (US)", MACHINE_SUPPORTS_SAVE ) // prototype?
