// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplan_v25_tables.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/v25.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*
Name        Board No      Maker         Game name
----------------------------------------------------------------------------
kbash       TP-023        Toaplan       Knuckle Bash
kbash2      bootleg       Toaplan       Knuckle Bash 2
*/

namespace {

class kbash_state : public driver_device
{
public:
	kbash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_shared_ram(*this, "shared_ram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_oki(*this, "oki")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void kbash(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	void kbash_68k_mem(address_map &map) ATTR_COLD;
	void kbash_v25_mem(address_map &map) ATTR_COLD;

	void kbash_oki_bankswitch_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }

	optional_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	bitmap_ind8 m_custom_priority_bitmap;
};

class kbash2_state : public kbash_state
{
public:
	kbash2_state(const machine_config &mconfig, device_type type, const char *tag)
		: kbash_state(mconfig, type, tag)
		, m_musicoki(*this, "musicoki")
	{ }

	void kbash2(machine_config &config) ATTR_COLD;

protected:
	void kbash2_68k_mem(address_map &map) ATTR_COLD;

	required_device<okim6295_device> m_musicoki;
};

void kbash_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}

u32 kbash_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void kbash_state::screen_vblank(int state)
{
	if (state) // rising edge
	{
		m_vdp->screen_eof();
	}
}



void kbash_state::kbash_oki_bankswitch_w(u8 data)
{
	m_oki->set_rom_bank(data & 1);
}


static INPUT_PORTS_START( base )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL_LOC(SW1)
	// Coinage on bit mask 0x00f0
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	// Per-game features on bit mask 0x00fc
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below
INPUT_PORTS_END


static INPUT_PORTS_START( kbash )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0001, "Discount" )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x70, 0x20, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0004, "100k only" )
	PORT_DIPSETTING(        0x0000, "100k and 400k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0000, "2" )
	PORT_DIPSETTING(        0x0020, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0020, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0020, "Europe, USA (Atari Games)" )   // European coinage
	PORT_CONFSETTING(       0x0010, "USA, Europe (Atari Games)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0060, DEF_STR( Southeast_Asia ) ) // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x0070, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x0080, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0090, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, DEF_STR( Europe ) ) // European coinage
//  PORT_CONFSETTING(        0x00b0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00c0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00d0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00e0, DEF_STR( Unused ) ) // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x00f0, DEF_STR( Unused ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kbashk )
	PORT_INCLUDE( kbash )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0000, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0000, "Japan (Taito license)" ) // Taito license
	PORT_CONFSETTING(       0x0010, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0020, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0060, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0070, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0080, DEF_STR( Japan ) ) // no Taito license
	PORT_CONFSETTING(       0x0090, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x00a0, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x00b0, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x00c0, DEF_STR( Hong_Kong ))
	PORT_CONFSETTING(       0x00d0, DEF_STR( Taiwan ))
	PORT_CONFSETTING(       0x00e0, DEF_STR( Southeast_Asia ))
	PORT_CONFSETTING(       0x00f0, DEF_STR( Unused ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kbash2 )
	PORT_INCLUDE( kbash )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x07, 0x02, SW1 )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000f,  0x0006, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0000, "Japan (Taito Corp.)" )
//  PORT_CONFSETTING(        0x0001, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x0002, DEF_STR( Unused ) ) // European coinage
	PORT_CONFSETTING(       0x0003, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0006, "Southeast Asia (Charterfield)" )   // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x0007, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x0009, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x000a, DEF_STR( Unused ) ) // European coinage
	PORT_CONFSETTING(       0x000b, DEF_STR( Korea ) )
//  PORT_CONFSETTING(        0x000c, DEF_STR( Hong_Kong ) )
//  PORT_CONFSETTING(        0x000d, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Southeast_Asia ) ) // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x000f, DEF_STR( Unused ) )
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



void kbash_state::kbash_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200fff).rw(FUNC(kbash_state::shared_ram_r), FUNC(kbash_state::shared_ram_w)).umask16(0x00ff);
	map(0x208010, 0x208011).portr("IN1");
	map(0x208014, 0x208015).portr("IN2");
	map(0x208018, 0x208019).portr("SYS");
	map(0x20801d, 0x20801d).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x700001).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));         // test bit 8
}


void kbash2_state::kbash2_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x10401f).ram();         // Sound related?
	map(0x200000, 0x200001).noprw();         // Left over from original code - Sound Number write, Status read
	map(0x200002, 0x200003).nopw();    // Left over from original code - Reset Sound
	map(0x200004, 0x200005).portr("DSWA");
	map(0x200008, 0x200009).portr("DSWB");
	map(0x20000c, 0x20000d).portr("JMPR");
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x200021, 0x200021).rw(m_musicoki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200025, 0x200025).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200029, 0x200029).w(FUNC(kbash2_state::kbash_oki_bankswitch_w));
	map(0x20002c, 0x20002d).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void kbash_state::kbash_v25_mem(address_map &map)
{
	map(0x00000, 0x007ff).ram().share(m_shared_ram);
	map(0x04000, 0x04001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x04002, 0x04002).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).rom().region("audiocpu", 0);
}


void kbash_state::kbash(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);         /* 16MHz Oscillator */
	m_maincpu->set_addrmap(AS_PROGRAM, &kbash_state::kbash_68k_mem);
	m_maincpu->reset_cb().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	/* ROM based v25 */
	v25_device &audiocpu(V25(config, m_audiocpu, 16_MHz_XTAL));         /* NEC V25 type Toaplan marked CPU ??? */
	audiocpu.set_addrmap(AS_PROGRAM, &kbash_state::kbash_v25_mem);
	audiocpu.set_decryption_table(toaplan_v25_tables::nitro_decryption_table);
	audiocpu.pt_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(kbash_state::screen_update));
	m_screen->screen_vblank().set(FUNC(kbash_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki, 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void kbash2_state::kbash2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);         /* 16MHz Oscillator */
	m_maincpu->set_addrmap(AS_PROGRAM, &kbash2_state::kbash2_68k_mem);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(kbash2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(kbash2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 16_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_musicoki, 16_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_musicoki->add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( kbash )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp023_01.bin", 0x000000, 0x080000, CRC(2965f81d) SHA1(46f2df30fa92c80ba5a37f75e756424e15534784) )

	/* Secondary CPU is a Toaplan marked chip, (TS-004-Dash  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )         /* Sound CPU code */
	ROM_LOAD( "tp023_02.bin", 0x0000, 0x8000, CRC(4cd882a1) SHA1(7199a5c384918f775f0815e09c46b2a58141814a) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "tp023_3.bin", 0x000000, 0x200000, CRC(32ad508b) SHA1(e473489beaf649d3e5236770eb043327e309850c) )
	ROM_LOAD( "tp023_5.bin", 0x200000, 0x200000, CRC(b84c90eb) SHA1(17a1531d884d9a9696d1b25d65f9155f02396e0e) )
	ROM_LOAD( "tp023_4.bin", 0x400000, 0x200000, CRC(e493c077) SHA1(0edcfb70483ad07206695d9283031b85cd198a36) )
	ROM_LOAD( "tp023_6.bin", 0x600000, 0x200000, CRC(9084b50a) SHA1(03b58278619524d2f09a4b1c152d5e057e792a56) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp023_7.bin", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END

ROM_START( kbashk )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp023_01.u52", 0x000000, 0x080000, CRC(099aefbc) SHA1(8daa0deffe221e1bb5a8744ced18c23ad319ffd3) ) // same label as parent?

	/* Secondary CPU is a Toaplan marked chip, (TS-004-Dash  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )         /* Sound CPU code */
	ROM_LOAD( "tp023_02.bin", 0x0000, 0x8000, CRC(4cd882a1) SHA1(7199a5c384918f775f0815e09c46b2a58141814a) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "tp023_3.bin", 0x000000, 0x200000, CRC(32ad508b) SHA1(e473489beaf649d3e5236770eb043327e309850c) )
	ROM_LOAD( "tp023_5.bin", 0x200000, 0x200000, CRC(b84c90eb) SHA1(17a1531d884d9a9696d1b25d65f9155f02396e0e) )
	ROM_LOAD( "tp023_4.bin", 0x400000, 0x200000, CRC(e493c077) SHA1(0edcfb70483ad07206695d9283031b85cd198a36) )
	ROM_LOAD( "tp023_6.bin", 0x600000, 0x200000, CRC(9084b50a) SHA1(03b58278619524d2f09a4b1c152d5e057e792a56) )

	ROM_REGION( 0x40000, "oki", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp023_7.bin", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END

// all labels handwritten. Given the AOU on the audio ROM, maybe a prototype destined to be showed there?
// GFX ROMs are on 4 riser boards. They are smaller but contain the same data as the final version.
// Only different ROMs are the main and audio CPU ones
ROM_START( kbashp )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2-19.u52", 0x000000, 0x080000, CRC(60dfdfec) SHA1(ca61433c8f7b1b765a699c375c946f113beeccc4) ) // actually 2/19

	/* Secondary CPU is a Toaplan marked chip, (NITRO TOA PLAN 509) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "aou-nb-sound.u34", 0x0000, 0x8000, CRC(26ba8fb1) SHA1(4259c4704f0fea0c8befa2e60a0838280b23a507) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "0.u1", 0x000000, 0x080000, CRC(1b87ffa5) SHA1(fbd5ac9e9635c1f5b1b896a3d504b827c0a99679) )
	ROM_LOAD( "2.u2", 0x080000, 0x080000, CRC(a411457e) SHA1(6b515e6524aa4fb1785d99556fefb0434368de84) )
	ROM_LOAD( "4.u3", 0x100000, 0x080000, CRC(631f770d) SHA1(0f0c11bc5549ed68d20dfc6ae51c3caec65aab88) )
	ROM_LOAD( "6.u4", 0x180000, 0x080000, CRC(5a46d262) SHA1(56d5180196b5acf76b700e627878998e88a21f3c) )
	ROM_LOAD( "8.u1", 0x200000, 0x080000, CRC(11b1c986) SHA1(05260c6cc5ab4239b52549e0dcda8853fc1fcd3a) )
	ROM_LOAD( "a.u2", 0x280000, 0x080000, CRC(4c4b47ce) SHA1(a41a27ac96bd9eb57bc4bd8b6592b70e86ad16d3) )
	ROM_LOAD( "c.u3", 0x300000, 0x080000, CRC(1ccc6a19) SHA1(d5735c2f075d81018021ec9e8642104227b67ace) )
	ROM_LOAD( "e.u4", 0x380000, 0x080000, CRC(731ad154) SHA1(78efce53000d170098b57342641299aacb7a82aa) )
	ROM_LOAD( "3.u1", 0x400000, 0x080000, CRC(7fbe0452) SHA1(c9b8c0d7126382fcdf8b5fa9a4466292954c88f7) )
	ROM_LOAD( "1.u2", 0x480000, 0x080000, CRC(6cd94e90) SHA1(9957ad69f8e80370dbf2cd863d0646241236f6b4) )
	ROM_LOAD( "5.u3", 0x500000, 0x080000, CRC(9cb4884e) SHA1(f596902b7740de4c262b4b18ac17eccca566ea77) )
	ROM_LOAD( "7.u4", 0x580000, 0x080000, CRC(53c2e0b6) SHA1(ee1128ad41ae3c68ef32d4211dd5205a9a5bb216) )
	ROM_LOAD( "g.u1", 0x600000, 0x080000, CRC(a63c795c) SHA1(30d3bb29cd73b31e233229f9304e3b87feaf01f3) )
	ROM_LOAD( "b.u2", 0x680000, 0x080000, CRC(32f8c39b) SHA1(a9029910c0b4fc3693081056a0afb9bcf9c0e699) )
	ROM_LOAD( "d.u3", 0x700000, 0x080000, CRC(40ac17d5) SHA1(140c67cf86ce545469fbe899b1f38c3a070908c9) )
	ROM_LOAD( "f.u4", 0x780000, 0x080000, CRC(2ca4eb83) SHA1(0d7c4242a82aba49cafd96ee5b051918d1b23b08) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "2m-nb-pcm.u40", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END

/*
Knuckle Bash 2
This is a hacked version of Knuckle Bash on bootleg/Korean/Chinese
hardware showing (C)Toaplan 1999 Licensed to Charterfield

PCB Layout
----------

|--------------------------------------------|
|UPC1241  EPROM  MECAT-S                     |
|  LM324                                     |
|        M6295  M6295                        |
| PAL   62256                      M5M51008  |
|       62256    MECAT-M           M5M51008  |
|        6116                      M5M51008  |
|J       6116         14.31818MHz  M5M51008  |
|A             68000                         |
|M                    16MHz                  |
|M                  PAL                      |
|A             PAL                           |
|        |-------|                           |
|        |ACTEL  |         PAL               |
|        |A40MX04|         PAL               |
|        |       |                           |
|   DSW1 |-------|         050917-10         |
|        |ACTEL  |                           |
|   DSW2 |A40MX04| MECAT-12                  |
|62256   |       |                           |
|62256   |-------| MECAT-34                  |
|--------------------------------------------|
Notes:
      68000 clock 16.000MHz
      M6295 clock 1.000MHz [16/16]. Sample rate (Hz) 16000000/16/132
      M5M51008 - Mitsubishi M5M51008 128k x8 SRAM (SOP32)
      62256    - 32k x8 SRAM
      6116     - 2k x8 SRAM
      VSync 60Hz
      HSync 15.68kHz
*/

ROM_START( kbash2 )
	ROM_REGION( 0x80000, "maincpu", 0 )         /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "mecat-m", 0x000000, 0x80000, CRC(bd2263c6) SHA1(eb794c0fc9c1fb4337114d48149283d42d22e4b3) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "mecat-34", 0x000000, 0x400000, CRC(6be7b37e) SHA1(13160ad0712fee932bb98cc226e651895b19228a) )
	ROM_LOAD( "mecat-12", 0x400000, 0x400000, CRC(49e46b1f) SHA1(d12b12696a8473eb34f3cd247ab060289a6c0e9c) )

	ROM_REGION( 0x80000, "oki", 0 )            /* ADPCM Music */
	ROM_LOAD( "mecat-s", 0x00000, 0x80000, CRC(3eb7adf4) SHA1(b0e6e99726b854858bd0e69eb77f12b9664b35e6) )

	ROM_REGION( 0x40000, "musicoki", 0 )            /* ADPCM Samples */
	ROM_LOAD( "eprom",   0x00000, 0x40000, CRC(31115cb9) SHA1(c79ea01bd865e2fc3aaab3ff05483c8fd27e5c98) )

	ROM_REGION( 0x10000, "user1", 0 )           /* ??? Some sort of table  - same as in pipibibi*/
	ROM_LOAD( "050917-10", 0x0000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) )
ROM_END

} // anonymous namespace

GAME( 1993, kbash,       0,        kbash,        kbash,      kbash_state, empty_init,    ROT0,   "Toaplan / Atari", "Knuckle Bash",                 MACHINE_SUPPORTS_SAVE ) // Atari license shown for some regions.
GAME( 1993, kbashk,      kbash,    kbash,        kbashk,     kbash_state, empty_init,    ROT0,   "Toaplan / Taito", "Knuckle Bash (Korean PCB)",    MACHINE_SUPPORTS_SAVE ) // Japan region has optional Taito license, maybe the original Japan release?
GAME( 1993, kbashp,      kbash,    kbash,        kbash,      kbash_state, empty_init,    ROT0,   "Toaplan / Taito", "Knuckle Bash (location test)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, kbash2,      0,        kbash2,       kbash2,     kbash2_state, empty_init,    ROT0,   "bootleg",         "Knuckle Bash 2 (bootleg)",  MACHINE_SUPPORTS_SAVE )
