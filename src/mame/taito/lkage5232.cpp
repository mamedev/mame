// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

The Legend of Kage (MSM5232 version)
(c)1984 TAITO CORPORATION.

Driver by Takahiro Nogi

* similar to taito/lkage and taito/wyvernf0 driver.

TODO:


***************************************************************************/

#include "emu.h"
#include "lkage5232.h"

#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


#define MAIN_CPU_CLOCK      (XTAL(12'000'000)/2)
#define SOUND_CPU_CLOCK     (XTAL(8'000'000)/2)
#define AUDIO_CLOCK         (XTAL(8'000'000)/2)


static INPUT_PORTS_START( lkage5232 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       /* table at 0x04b8 */
	PORT_DIPSETTING(    0x03, "200k 700k 500k+" )
	PORT_DIPSETTING(    0x02, "200k 900k 700k+" )
	PORT_DIPSETTING(    0x01, "300k 1000k 700k+" )
	PORT_DIPSETTING(    0x00, "300k 1300k 1000k+" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Initial Season" )
	PORT_DIPSETTING(    0x02, "Spring" )
	PORT_DIPSETTING(    0x00, "Winter" )                    /* same as if you saved the princess twice ("HOWEVER ...") */
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, "1985" )
	PORT_DIPSETTING(    0x20, "MCMLXXXIV" )                 /* 1984(!) */
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4),RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4),RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			64+7, 64+6, 64+5, 64+4, 64+3, 64+2, 64+1, 64+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			128+0*8, 128+1*8, 128+2*8, 128+3*8, 128+4*8, 128+5*8, 128+6*8, 128+7*8 },
	32*8
};

static GFXDECODE_START( gfx_lkage5232 )
	GFXDECODE_ENTRY( "gfx", 0x0000, tile_layout,  /*128*/0, 64 )
	GFXDECODE_ENTRY( "gfx", 0x0000, sprite_layout,  0, 16 )
GFXDECODE_END


void lkage5232_state::vreg_w(offs_t offset, uint8_t data)
{
	m_vreg[offset] = data;
}


uint8_t lkage5232_state::unk_f0e1_r(offs_t offset)
{
	// Protecton?
	return 0xff;
}

uint8_t lkage5232_state::exrom_data_r(offs_t offset)
{
	uint16_t offs;
	uint8_t data;

	offs = ((m_exrom_offs[1] & 0x3f) << 8) + (m_exrom_offs[0] & 0xff);
	data = memregion("user1")->base()[offs] & 0xff;

	if (offset != 0)
			return 0xff;
		else
			return data;
}

void lkage5232_state::exrom_offset_w(offs_t offset, uint8_t data)
{
	m_exrom_offs[offset] = data;
}


/***************************************************************************/
void lkage5232_state::lkage5232_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xefff).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf003).ram().share("vreg"); /* video registers */
	map(0xf060, 0xf060).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(FUNC(lkage5232_state::sound_command_w));
	map(0xf061, 0xf061).nopw().r(FUNC(lkage5232_state::sound_status_r));
	map(0xf063, 0xf063).noprw(); /* unknown */
	map(0xf080, 0xf080).portr("DSW1");
	map(0xf081, 0xf081).portr("DSW2");
	map(0xf082, 0xf082).portr("DSW3");
	map(0xf083, 0xf083).portr("SYSTEM");
	map(0xf084, 0xf084).portr("P1");
	map(0xf086, 0xf086).portr("P2");
	map(0xf0a0, 0xf0a1).r(FUNC(lkage5232_state::exrom_data_r)).w(FUNC(lkage5232_state::exrom_offset_w)); /* Extend ROM Read */
	map(0xf0a2, 0xf0a3).noprw(); /* unknown */
	map(0xf0c0, 0xf0c5).ram().share("scroll");
	map(0xf0e0, 0xf0e0).noprw();
	map(0xf0e1, 0xf0e1).nopw().r(FUNC(lkage5232_state::unk_f0e1_r)); /* unknown */
	map(0xf100, 0xf1ff).ram().share("spriteram");
	map(0xf400, 0xffff).ram().w(FUNC(lkage5232_state::videoram_w)).share("videoram");
}


//-------------------------------------------------
TIMER_CALLBACK_MEMBER(lkage5232_state::sound_nmi_callback)
{
	if (m_sound_nmi_enable)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	else
		m_pending_nmi = 1;
}

void lkage5232_state::sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(lkage5232_state::sound_nmi_callback),this), data);
}

void lkage5232_state::sound_nmi_disable_w(uint8_t data)
{
	m_sound_nmi_enable = 0;
}

void lkage5232_state::sound_nmi_enable_w(uint8_t data)
{
	m_sound_nmi_enable = 1;
	if (m_pending_nmi)
	{
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_pending_nmi = 0;
	}
}

uint8_t lkage5232_state::sound_status_r(offs_t offset)
{
	return 0xff;
}

uint8_t lkage5232_state::sound_unk_e000_r(offs_t offset)
{
	return 0xff;
}


void lkage5232_state::lkage5232_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8001).w("ay1", FUNC(ym2149_device::address_data_w));
	map(0x8002, 0x8003).w("ay2", FUNC(ym2149_device::address_data_w));
	map(0x8010, 0x801d).w("msm", FUNC(msm5232_device::write));
	map(0x8020, 0x8020).nopw(); // ?
	map(0xc000, 0xc000).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
	map(0xc001, 0xc001).nopr().w(FUNC(lkage5232_state::sound_nmi_enable_w));
	map(0xc002, 0xc002).w(FUNC(lkage5232_state::sound_nmi_disable_w));
//  map(0xc003, 0xc003).w("dac", FUNC(dac_byte_interface::data_w));
	map(0xc003, 0xc003).nopw();
	map(0xe000, 0xe000).r(FUNC(lkage5232_state::sound_unk_e000_r));
}


void lkage5232_state::machine_start()
{
	save_item(NAME(m_bg_tile_bank));
	save_item(NAME(m_fg_tile_bank));
	save_item(NAME(m_tx_tile_bank));
	save_item(NAME(m_sprite_dx));
}

void lkage5232_state::machine_reset()
{
	m_bg_tile_bank = m_fg_tile_bank = m_tx_tile_bank =0;

	sound_nmi_disable_w(0);
}

void lkage5232_state::init_lkage5232()
{
	m_sprite_dx = 0;
}


void lkage5232_state::lkage5232(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lkage5232_state::lkage5232_map);
	m_maincpu->set_vblank_int("screen", FUNC(lkage5232_state::irq0_line_hold));

	Z80(config, m_audiocpu, SOUND_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &lkage5232_state::lkage5232_sound_map);
	m_audiocpu->set_periodic_int(FUNC(lkage5232_state::irq0_line_hold), attotime::from_hz(60));

	/* 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs */
	config.set_maximum_quantum(attotime::from_hz(6000));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(2*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(lkage5232_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lkage5232);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2149(config, "ay1", 2_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.18);
	YM2149(config, "ay2", 2_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.18);

	msm5232_device &msm(MSM5232(config, "msm", 2_MHz_XTAL));
	msm.set_capacitors(0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6);
	msm.add_route(0, "mono", 1.35);
	msm.add_route(1, "mono", 1.35);
	msm.add_route(2, "mono", 1.35);
	msm.add_route(3, "mono", 1.35);
	msm.add_route(4, "mono", 1.35);
	msm.add_route(5, "mono", 1.35);
	msm.add_route(6, "mono", 1.35);
	msm.add_route(7, "mono", 1.35);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}


ROM_START( lkage5232 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF ) /* Z80 code (main CPU) */
	ROM_LOAD( "a51_11-2.ic", 0x0000, 0x4000, CRC(540fdb1f) SHA1(11d2a5b56d6d72458816aaf7687e490126b468cc) ) // lkage5232 ok
	ROM_LOAD( "a51_12-2.ic", 0x4000, 0x4000, CRC(a625a4b8) SHA1(417e7590f98eadc71cbed749350d6d3a1c1fd413) ) // lkage5232 ok
	ROM_LOAD( "a51_13-2.ic", 0x8000, 0x4000, CRC(aba8c6a3) SHA1(6723138f54a06d3e4719a43e8e3f11b3cabfb6f7) ) // lkage5232 ok
	ROM_LOAD( "a51_10-2.ic", 0xc000, 0x2000, CRC(f6243d5c) SHA1(7b2810afe9c128a290f15c6ea1a6a1c2757ddf55) ) // lkage5232 ok

	// lkage5232
	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound CPU) */
	ROM_LOAD( "a51_01-1.ic", 0x0000, 0x4000, CRC(03c818ba) SHA1(80604726c647495ab76870806cd1fb448cffe34d) ) // lkage5232 ok
	ROM_FILL(                0xe000, 0x2000, 0xff ) // diagnostics ROM

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "a51_03-1.ic", 0x0000, 0x2000, CRC(99847f0a) SHA1(34ea492e82845d0366bd755ddf1cad7f574d867a) ) // tile // lkage5232 ok
	ROM_LOAD( "a51_07-1.ic", 0x2000, 0x2000, CRC(c9d01e5b) SHA1(16d689ccc9c3cb16e6b4d85f8e50386c78c439e5) ) // spr  // lkage5232 ok
	ROM_LOAD( "a51_02-1.ic", 0x4000, 0x2000, CRC(28bbf964) SHA1(67fb767549d7326133c630f424703abe2b14273d) ) // tile // lkage5232 ok
	ROM_LOAD( "a51_06-1.ic", 0x6000, 0x2000, CRC(d16c7c95) SHA1(f3cfc995cc072311b3bd831b69ccb229e2734f53) ) // spr  // lkage5232 ok
	ROM_LOAD( "a51_05-1.ic", 0x8000, 0x2000, CRC(38bb3ad0) SHA1(9c24d705e55acaaa99fbb39e06486ca932bda796) ) // tile // lkage5232 ok
	ROM_LOAD( "a51_09-1.ic", 0xa000, 0x2000, CRC(40fd3d86) SHA1(f92156e5e44483b2683457166cb5b9cfc7fbbf14) ) // spr  // lkage5232 ok
	ROM_LOAD( "a51_04-1.ic", 0xc000, 0x2000, CRC(8e132cc6) SHA1(c7b196e6b8c3b6841a1f4ca0904597085a53cc25) ) // tile // lkage5232 ok
	ROM_LOAD( "a51_08-1.ic", 0xe000, 0x2000, CRC(2ab68af8) SHA1(6dd91311f2344936590577440898da5f26e35880) ) // spr  // lkage5232 ok

	ROM_REGION( 0x4000, "user1", 0 ) /* data */
	ROM_LOAD( "a51_14.ic",   0x0000, 0x4000, CRC(493e76d8) SHA1(13c6160edd94ba2801fd89bb33bcae3a1e3454ff) ) // lkage ok
ROM_END


//    year  �@�@ name parent    machine      input            class            init    rot              company                                         fullname  flags
GAME( 1984, lkage5232,     0, lkage5232, lkage5232, lkage5232_state, init_lkage5232,  ROT0, "Taito Corporation", "The Legend of Kage (MSM5232 ver)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
