// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Mad Motor                             (c) 1989 Mitchell Corporation

  But it's really a Data East game..  Bad Dudes era graphics hardware with
  Dark Seal era sound hardware.  Maybe a license for a specific territory?

  "This game is developed by Mitchell, but they entrusted PCB design and some
  routines to Data East."

  Emulation by Bryan McPhail, mish@tendril.co.uk

  Notes:  Playfield 3 can change size between 512x1024 and 2048x256

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"
#include "decbac06.h"
#include "decmxc06.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class madmotor_state : public driver_device
{
public:
	madmotor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tilegen(*this, "tilegen%u", 1),
		m_spritegen(*this, "spritegen"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void madmotor(machine_config &config);

	void init_madmotor();

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void madmotor_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<h6280_device> m_audiocpu;
	required_device_array<deco_bac06_device, 3> m_tilegen;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
};


/******************************************************************************/


void madmotor_state::madmotor_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x180000, 0x180007).w("tilegen1", FUNC(deco_bac06_device::pf_control_0_w));                          /* text layer */
	map(0x180010, 0x180017).w("tilegen1", FUNC(deco_bac06_device::pf_control_1_w));
	map(0x184000, 0x18407f).rw("tilegen1", FUNC(deco_bac06_device::pf_colscroll_r), FUNC(deco_bac06_device::pf_colscroll_w));
	map(0x184080, 0x1843ff).ram();
	map(0x184400, 0x1847ff).rw("tilegen1", FUNC(deco_bac06_device::pf_rowscroll_r), FUNC(deco_bac06_device::pf_rowscroll_w));
	map(0x188000, 0x189fff).rw("tilegen1", FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x18c000, 0x18c001).noprw();
	map(0x190000, 0x190007).w("tilegen2", FUNC(deco_bac06_device::pf_control_0_w));                          /* text layer */
	map(0x190010, 0x190017).w("tilegen2", FUNC(deco_bac06_device::pf_control_1_w));
	map(0x198000, 0x1987ff).rw("tilegen2", FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x19c000, 0x19c001).nopr();
	map(0x1a0000, 0x1a0007).w("tilegen3", FUNC(deco_bac06_device::pf_control_0_w));                          /* text layer */
	map(0x1a0010, 0x1a0017).w("tilegen3", FUNC(deco_bac06_device::pf_control_1_w));
	map(0x1a4000, 0x1a4fff).rw("tilegen3", FUNC(deco_bac06_device::pf_data_r), FUNC(deco_bac06_device::pf_data_w));
	map(0x3e0000, 0x3e3fff).ram();
	map(0x3e8000, 0x3e87ff).ram().share("spriteram");
	map(0x3f0000, 0x3f07ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x3f8002, 0x3f8003).portr("P1_P2");
	map(0x3f8004, 0x3f8005).portr("DSW");
	map(0x3f8006, 0x3f8007).portr("SYSTEM");
	map(0x3fc005, 0x3fc005).w("soundlatch", FUNC(generic_latch_8_device::write));
}

/******************************************************************************/

/* Physical memory map (21 bits) */
void madmotor_state::sound_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x100000, 0x100001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x110000, 0x110001).rw("ym2", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x120000, 0x120001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x130000, 0x130001).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x140000, 0x140001).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x1f0000, 0x1f1fff).ram();
}

/******************************************************************************/

static INPUT_PORTS_START( madmotor )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )   /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )   /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	RGN_FRAC(1,2),
	4,      /* 4 bits per pixel  */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), RGN_FRAC(0,2)+8, RGN_FRAC(0,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), RGN_FRAC(0,2)+8, RGN_FRAC(0,2) },
	{ STEP8(16*8*2,1), STEP8(0,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static GFXDECODE_START( gfx_madmotor )
	GFXDECODE_ENTRY( "char",    0, charlayout,   0, 16 ) /* Characters 8x8 */
	GFXDECODE_ENTRY( "tiles1",  0, tilelayout, 512, 16 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( "tiles2",  0, tilelayout, 768, 16 ) /* Tiles 16x16 */
GFXDECODE_END

static GFXDECODE_START( gfx_madmotor_spr )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 256, 16 ) /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

uint32_t madmotor_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	m_tilegen[2]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, 0x800/2);
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	return 0;
}

/******************************************************************************/

void madmotor_state::madmotor(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000); /* Custom chip 59, 24 MHz crystal */
	m_maincpu->set_addrmap(AS_PROGRAM, &madmotor_state::madmotor_map);
	m_maincpu->set_vblank_int("screen", FUNC(madmotor_state::irq6_line_hold)); /* VBL */

	H6280(config, m_audiocpu, 8053000/2); /* Custom chip 45, Crystal near CPU is 8.053 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &madmotor_state::sound_map);
	m_audiocpu->add_route(ALL_OUTPUTS, "mono", 0); // internal sound unused

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */ /* frames per second, vblank duration taken from Burger Time */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(madmotor_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_madmotor);
	PALETTE(config, "palette").set_format(palette_device::xBGR_444, 1024);

	DECO_BAC06(config, m_tilegen[0], 0);
	m_tilegen[0]->set_gfx_region_wide(0, 0, 0);
	m_tilegen[0]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[1], 0);
	m_tilegen[1]->set_gfx_region_wide(0, 1, 0);
	m_tilegen[1]->set_gfxdecode_tag(m_gfxdecode);

	DECO_BAC06(config, m_tilegen[2], 0);
	m_tilegen[2]->set_gfx_region_wide(0, 2, 1);
	m_tilegen[2]->set_gfxdecode_tag(m_gfxdecode);

	DECO_MXC06(config, m_spritegen, 0, "palette", gfx_madmotor_spr);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	YM2203(config, "ym1", 21470000/6).add_route(ALL_OUTPUTS, "mono", 0.40);

	ym2151_device &ym2(YM2151(config, "ym2", 21470000/6));
	ym2.irq_handler().set_inputline(m_audiocpu, 1); /* IRQ2 */
	ym2.add_route(0, "mono", 0.45);
	ym2.add_route(1, "mono", 0.45);

	OKIM6295(config, "oki1", 1023924, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // clock frequency & pin 7 not verified

	OKIM6295(config, "oki2", 2047848, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.25); // clock frequency & pin 7 not verified
}

/******************************************************************************/

/* ROM names all corrected as per exact checksum matching PCB - system11 */
ROM_START( madmotor )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "02-2.b4", 0x00000, 0x20000, CRC(50b554e0) SHA1(e33d0ab5464ab5ff394dd630536ac83baf0aa2c9) )
	ROM_LOAD16_BYTE( "00-2.b1", 0x00001, 0x20000, CRC(2d6a1b3f) SHA1(fa7058bf907becac56ed9938c5643aaefdf7a2c0) )
	ROM_LOAD16_BYTE( "03-2.b6", 0x40000, 0x20000, CRC(442a0a52) SHA1(86bb5470d5653d125481250f778c632371dddad8) )
	ROM_LOAD16_BYTE( "01-2.b3", 0x40001, 0x20000, CRC(e246876e) SHA1(648dca8bab001cfb42618081bbc1efa14118743e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "14.l7",    0x00000, 0x10000, CRC(1c28a7e5) SHA1(ed30d0a5a8a079677bd34b6d98ab1b15b934b30f) )

	ROM_REGION( 0x020000, "char", 0 )
	ROM_LOAD16_BYTE( "04.a9",     0x000000, 0x10000, CRC(833ca3ab) SHA1(7a3e7ebecc1596d2e487595369ad9ba54ced5bfb) )    /* chars */
	ROM_LOAD16_BYTE( "05.a11",    0x000001, 0x10000, CRC(a691fbfe) SHA1(c726a4c15d599feb6883d9b643453e7028fa16d6) )

	ROM_REGION( 0x040000, "tiles1", 0 )
	ROM_LOAD16_BYTE( "10.a19",    0x000000, 0x20000, CRC(9dbf482b) SHA1(086e9170d577e502604c180f174fbce53a1e20e5) )    /* tiles */
	ROM_LOAD16_BYTE( "11.a21",    0x000001, 0x20000, CRC(593c48a9) SHA1(1158888f6b836253b8ae9db9b8e352f289b2e815) )

	ROM_REGION( 0x080000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "06.a13",    0x000000, 0x20000, CRC(448850e5) SHA1(6a44a42738cf6a55b4bec807e0a3939a42b36793) )    /* tiles */
	ROM_LOAD16_BYTE( "07.a14",    0x040000, 0x20000, CRC(ede4d141) SHA1(7b847372bac043aa397aa5c274f90b9193de9176) )
	ROM_LOAD16_BYTE( "08.a16",    0x000001, 0x20000, CRC(c380e5e5) SHA1(ec87a94e7948b84c96b1577f5a8caebc56e38a94) )
	ROM_LOAD16_BYTE( "09.a18",    0x040001, 0x20000, CRC(1ee3326a) SHA1(bd03e5c4a2e7689260e6cc67288e71ef13f05a4b) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "15.h11",    0x000000, 0x20000, CRC(90ae9f74) SHA1(806f96fd08fca1beeeaefe3c0fac1991410aa9c4) )    /* sprites */
	ROM_LOAD16_BYTE( "16.h13",    0x040000, 0x20000, CRC(e96ac815) SHA1(a2b22a29ad0a4f144bb09299c454dc7a842a5318) )
	ROM_LOAD16_BYTE( "17.h14",    0x000001, 0x20000, CRC(abad9a1b) SHA1(3cec6b4ef925205efe4a8fb28e08eb58e3ba4019) )
	ROM_LOAD16_BYTE( "18.h16",    0x040001, 0x20000, CRC(96d8d64b) SHA1(54ce87fe2b14b574176d2a1d2b86057b9cd10883) )
	ROM_LOAD16_BYTE( "19.j13",    0x080000, 0x20000, CRC(cbd8c9b8) SHA1(5e86c0298b3eea06920121eecb70e5bee705addf) )
	ROM_LOAD16_BYTE( "20.j14",    0x0c0000, 0x20000, CRC(47f706a8) SHA1(bd4fe499710f8905eb4b8d1ca990f2908feb95e1) )
	ROM_LOAD16_BYTE( "21.j16",    0x080001, 0x20000, CRC(9c72d364) SHA1(9290e463273fa1f921279f1bab808d91d3aa9648) )
	ROM_LOAD16_BYTE( "22.j18",    0x0c0001, 0x20000, CRC(1e78aa60) SHA1(f5f58ee6f5efe56e72623e57ce27884551e09bd9) )

	ROM_REGION( 0x40000, "oki1", 0 )    /* ADPCM samples */
	ROM_LOAD( "12.h1",    0x00000, 0x20000, CRC(c202d200) SHA1(8470654923a0e8780dad678f5745f8e3e3be08b2) )

	ROM_REGION( 0x40000, "oki2", 0 )    /* ADPCM samples */
	ROM_LOAD( "13.h3",    0x00000, 0x20000, CRC(cc4d65e9) SHA1(b9bcaa52c570f94d2f2e5dd84c94773cc4115442) )
ROM_END

/******************************************************************************/

void madmotor_state::init_madmotor()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0x00000; i < 0x80000; i++)
	{
		rom[i] = (rom[i] & 0xdb) | ((rom[i] & 0x04) << 3) | ((rom[i] & 0x20) >> 3);
		rom[i] = (rom[i] & 0x7e) | ((rom[i] & 0x01) << 7) | ((rom[i] & 0x80) >> 7);
	}
}

} // anonymous namespace


	/* The title screen is undated, but it's (c) 1989 Data East at 0xefa0 */
GAME( 1989, madmotor, 0, madmotor, madmotor, madmotor_state, init_madmotor, ROT0, "Mitchell", "Mad Motor (prototype)", MACHINE_SUPPORTS_SAVE )
