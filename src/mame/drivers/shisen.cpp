// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Tamtex Shisensho / Match It

driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "includes/shisen.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/rstbuf.h"
#include "sound/ym2151.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


void shisen_state::shisen_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc800, 0xcaff).ram().w(FUNC(shisen_state::paletteram_w)).share("paletteram");
	map(0xd000, 0xdfff).ram().w(FUNC(shisen_state::videoram_w)).share("videoram");
	map(0xe000, 0xffff).ram();
}


WRITE8_MEMBER(shisen_state::coin_w)
{
	if ((data & 0xf9) != 0x01) logerror("coin ctrl = %02x\n",data);

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
}

void shisen_state::shisen_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1").w(FUNC(shisen_state::coin_w));
	map(0x01, 0x01).portr("DSW2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).portr("P1").w(FUNC(shisen_state::bankswitch_w));
	map(0x03, 0x03).portr("P2");
	map(0x04, 0x04).portr("COIN");
}


void shisen_state::shisen_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xfd00, 0xffff).ram();
}

void shisen_state::shisen_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).w(m_audio, FUNC(m72_audio_device::shisen_sample_addr_w));
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r));
}


static INPUT_PORTS_START( shisen )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Timer" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) ) PORT_CONDITION("DSW2",0x04,EQUALS,0x04) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW2",0x04,NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW2",0x04,NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x08, 0x08, "Nude Pictures" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Women Select" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Play Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x40, "2 Player" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( matchit )
	PORT_INCLUDE( shisen )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	4096*8,
	4,
	{ 0, 4, 0x80000*8+0, 0x80000*8+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};


static GFXDECODE_START( gfx_shisen )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,  0, 16 )
GFXDECODE_END



void shisen_state::shisen(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6000000);   /* 6 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &shisen_state::shisen_map);
	m_maincpu->set_addrmap(AS_IO, &shisen_state::shisen_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(shisen_state::irq0_line_hold));

	z80_device &soundcpu(Z80(config, "soundcpu", 3579645));
	soundcpu.set_addrmap(AS_PROGRAM, &shisen_state::shisen_sound_map);
	soundcpu.set_addrmap(AS_IO, &shisen_state::shisen_sound_io_map);
	soundcpu.set_periodic_int(FUNC(shisen_state::nmi_line_pulse), attotime::from_hz(128*55));  /* clocked by V1? (Vigilante) */
								/* IRQs are generated by main Z80 and YM2151 */
	soundcpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(55);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(shisen_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shisen);
	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq", 0).int_callback().set_inputline("soundcpu", 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3579545));
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(0, "lspeaker", 0.5);
	ymsnd.add_route(1, "rspeaker", 0.5);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25).add_route(ALL_OUTPUTS, "rspeaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sichuan2 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k+128k for main CPU */
	ROM_LOAD( "ic06.06",      0x00000, 0x10000, CRC(98a2459b) SHA1(42102cf2921f80be7600b11aba63538e3b3858ec) )
	ROM_RELOAD(               0x10000, 0x10000 )
	ROM_LOAD( "ic07.03",      0x20000, 0x10000, CRC(0350f6e2) SHA1(c683571969c0e4c66eb316a1bc580759db02bbfc) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic01.01",      0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic08.04",      0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "ic09.05",      0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "ic12.08",      0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "ic13.09",      0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "ic14.10",      0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "ic15.11",      0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "ic16.12",      0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "ic17.13",      0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "ic18.14",      0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "ic19.15",      0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "ic20.16",      0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "ic21.17",      0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "ic22.18",      0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "ic23.19",      0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "ic10.06",      0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "ic11.07",      0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", 0 ) /* samples */
	ROM_LOAD( "ic02.02",      0x00000, 0x10000, CRC(92f0093d) SHA1(530b924aa991283045577d03524dfc7eacf1be49) )
	ROM_LOAD( "ic03.03",      0x10000, 0x10000, CRC(116a049c) SHA1(656c0d1d7f945c5f5637892721a58421b682fd01) )
	ROM_LOAD( "ic04.04",      0x20000, 0x10000, CRC(6840692b) SHA1(f6f7b063ecf7206e172843515be38376f8845b42) )
	ROM_LOAD( "ic05.05",      0x30000, 0x10000, CRC(92ffe22a) SHA1(19dcaf6e25bb7498d4ab19fa0a63f3326b9bff8f) )
ROM_END

ROM_START( sichuan2a )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k+128k for main CPU */
	ROM_LOAD( "sichuan.a6",   0x00000, 0x10000, CRC(f8ac05ef) SHA1(cd20e5239d73264f1323ba6b1e35934685852ba1) )
	ROM_RELOAD(               0x10000, 0x10000 )
	ROM_LOAD( "ic07.03",      0x20000, 0x10000, CRC(0350f6e2) SHA1(c683571969c0e4c66eb316a1bc580759db02bbfc) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic01.01",      0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic08.04",      0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "ic09.05",      0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "ic12.08",      0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "ic13.09",      0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "ic14.10",      0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "ic15.11",      0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "ic16.12",      0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "ic17.13",      0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "ic18.14",      0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "ic19.15",      0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "ic20.16",      0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "ic21.17",      0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "ic22.18",      0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "ic23.19",      0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "ic10.06",      0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "ic11.07",      0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", 0 ) /* samples */
	ROM_LOAD( "ic02.02",      0x00000, 0x10000, CRC(92f0093d) SHA1(530b924aa991283045577d03524dfc7eacf1be49) )
	ROM_LOAD( "ic03.03",      0x10000, 0x10000, CRC(116a049c) SHA1(656c0d1d7f945c5f5637892721a58421b682fd01) )
	ROM_LOAD( "ic04.04",      0x20000, 0x10000, CRC(6840692b) SHA1(f6f7b063ecf7206e172843515be38376f8845b42) )
	ROM_LOAD( "ic05.05",      0x30000, 0x10000, CRC(92ffe22a) SHA1(19dcaf6e25bb7498d4ab19fa0a63f3326b9bff8f) )
ROM_END

ROM_START( shisen )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k+128k for main CPU */
	ROM_LOAD( "a-27-a.rom",   0x00000, 0x20000, CRC(de2ecf05) SHA1(7256c5587f92db10a52c43001e3236f3be3df5df) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic01.01",      0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic08.04",      0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "ic09.05",      0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "ic12.08",      0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "ic13.09",      0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "ic14.10",      0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "ic15.11",      0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "ic16.12",      0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "ic17.13",      0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "ic18.14",      0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "ic19.15",      0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "ic20.16",      0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "ic21.17",      0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "ic22.18",      0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "ic23.19",      0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "ic10.06",      0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "ic11.07",      0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", 0 ) /* samples */
	ROM_LOAD( "ic02.02",      0x00000, 0x10000, CRC(92f0093d) SHA1(530b924aa991283045577d03524dfc7eacf1be49) )
	ROM_LOAD( "ic03.03",      0x10000, 0x10000, CRC(116a049c) SHA1(656c0d1d7f945c5f5637892721a58421b682fd01) )
	ROM_LOAD( "ic04.04",      0x20000, 0x10000, CRC(6840692b) SHA1(f6f7b063ecf7206e172843515be38376f8845b42) )
	ROM_LOAD( "ic05.05",      0x30000, 0x10000, CRC(92ffe22a) SHA1(19dcaf6e25bb7498d4ab19fa0a63f3326b9bff8f) )
ROM_END


/*
Match It
Tamtex, 1989


CPU   : Z80A
SOUND : Z80A, YM2151, YM3012, LM324, GL4558
DIPSW : 8 position (x2)
XTAL  : 3.579545MHz, 24.000MHz
RAM   : GM76C88-12D (=6264, x3), 6116 (x3)
PROMs : None
PALs  : PAL16L8 (x3)

ROMs  : (All ROMs type 27C512)

1.2C     Sound Program

2.11D  \
3.11C  / Main Program

4.3J   \
5.4J   |
6.5J   |
7.6J   |
8.1L   |
9.2L   |
10.3L  |
11.5L  | GFX
12.6L  |
13.7L  |
14.8L  |
15.10L |
16.11L |
17.12L |
18.13L |
19.14L /
*/

ROM_START( matchit )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k+128k for main CPU */
	ROM_LOAD( "2.11d",      0x00000, 0x10000, CRC(299815f7) SHA1(dd25f69d3c825e12e5c2e24b5bbfda9c39400345) )
	ROM_RELOAD(               0x10000, 0x10000 )
	ROM_LOAD( "ic07.03",      0x20000, 0x10000, CRC(0350f6e2) SHA1(c683571969c0e4c66eb316a1bc580759db02bbfc) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic01.01",      0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "ic08.04",      0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "ic09.05",      0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "ic12.08",      0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "ic13.09",      0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "ic14.10",      0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "ic15.11",      0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "ic16.12",      0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "ic17.13",      0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "ic18.14",      0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "ic19.15",      0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "ic20.16",      0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "ic21.17",      0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "ic22.18",      0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "ic23.19",      0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "ic10.06",      0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "ic11.07",      0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* no samples on this board */
ROM_END

GAME( 1989, matchit,  0,         shisen,   matchit, shisen_state, empty_init, ROT0, "Tamtex",  "Match It",                         MACHINE_SUPPORTS_SAVE )
GAME( 1989, shisen,   matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "Tamtex",  "Shisensho - Joshiryo-Hen (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, sichuan2, matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "hack",    "Sichuan II (hack, set 1)",         MACHINE_SUPPORTS_SAVE )
GAME( 1989, sichuan2a,matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "hack",    "Sichuan II (hack, set 2)",         MACHINE_SUPPORTS_SAVE )
