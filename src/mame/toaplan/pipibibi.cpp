// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

/*
* Name        Board No      Maker         Game name
----------------------------------------------------------------------------
pipibibs    TP-025        Toaplan       Pipi & Bibis / Whoopee!! (set 1)
pipibibsa   TP-025        Toaplan       Pipi & Bibis / Whoopee!! (set 2)
pipibibsp   TP-025        Toaplan       Pipi & Bibis / Whoopee!! (Prototype)
pipibibsbl  bootleg       Toaplan       Pipi & Bibis / Whoopee!! (based of the prototype)

TODO:
    - move bootlegs to oneshot.cpp driver
*/

namespace {

class pipibibi_state : public driver_device
{
public:
	pipibibi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_shared_ram(*this, "shared_ram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void pipibibs(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }

	void pipibibs_68k_mem(address_map &map) ATTR_COLD;
	void pipibibs_sound_z80_mem(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	required_device<m68000_base_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	bitmap_ind8 m_custom_priority_bitmap;
};

class pipibibi_bootleg_state : public pipibibi_state
{
public:
	pipibibi_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: pipibibi_state(mconfig, type, tag)
	{ }

	void pipibibsbl(machine_config &config) ATTR_COLD;

	void init_pipibibsbl() ATTR_COLD;

private:
	void cpu_space_pipibibsbl_map(address_map &map) ATTR_COLD;
	void pipibibi_bootleg_68k_mem(address_map &map) ATTR_COLD;
};

void pipibibi_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}


u32 pipibibi_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void pipibibi_state::screen_vblank(int state)
{
	if (state) // rising edge
	{
		m_vdp->screen_eof();
	}
}



static INPUT_PORTS_START( base )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

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

static INPUT_PORTS_START( pipibibs )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000f - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x06, 0x06, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "200k and every 300k" )
	PORT_DIPSETTING(        0x0004, "150k and every 200k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x0008,  0x0000, "Nudity" )          //PORT_CONFLOCATION("JP:!1")
	PORT_CONFSETTING(       0x0008, DEF_STR( Low ) )
	PORT_CONFSETTING(       0x0000, "High, but censored" )
	PORT_CONFNAME( 0x0007,  0x0006, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2")
	PORT_CONFSETTING(       0x0006, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0004, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( Asia ) )
	PORT_CONFSETTING(       0x0002, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x0003, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0005, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x0007, "Europe (Nova Apparate GMBH & Co.)" )
INPUT_PORTS_END

static INPUT_PORTS_START( pipibibsp )
	PORT_INCLUDE( pipibibs )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000d - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x80000, 0x80000, SW1 )

	PORT_MODIFY("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x80007, 0x00002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,FAKE:!1")
	PORT_CONFSETTING(       0x00002, DEF_STR( World ) )
	PORT_CONFSETTING(       0x80005, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00004, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, "Japan (Ryouta Kikaku)" )
	PORT_CONFSETTING(       0x00001, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x00007, "World (Ryouta Kikaku)" )
INPUT_PORTS_END

static INPUT_PORTS_START( pipibibsbl )
	PORT_INCLUDE( pipibibs )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0002,   0x0000, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:!2")  // In Test Mode, it shows as Normal/Invert Screen - HW doesn't support it
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0002, DEF_STR( On ) )
	// Various features on bit mask 0x000d - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x80000, 0x80000, SW1 )

	PORT_MODIFY("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x80007, 0x00002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,FAKE:!1")
	PORT_CONFSETTING(       0x00002, DEF_STR( World ) )
	PORT_CONFSETTING(       0x80005, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00004, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, "Japan (Ryouta Kikaku)" )
	PORT_CONFSETTING(       0x00001, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x00007, "World (Ryouta Kikaku)" )
INPUT_PORTS_END

void pipibibi_state::pipibibs_68k_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x082fff).ram();
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x14000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x190000, 0x190fff).rw(FUNC(pipibibi_state::shared_ram_r), FUNC(pipibibi_state::shared_ram_w)).umask16(0x00ff);
	map(0x19c01d, 0x19c01d).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
	map(0x19c020, 0x19c021).portr("DSWA");
	map(0x19c024, 0x19c025).portr("DSWB");
	map(0x19c028, 0x19c029).portr("JMPR");
	map(0x19c02c, 0x19c02d).portr("SYS");
	map(0x19c030, 0x19c031).portr("IN1");
	map(0x19c034, 0x19c035).portr("IN2");
}

// odd scroll registers
void pipibibi_bootleg_state::pipibibi_bootleg_68k_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x082fff).ram();
	map(0x083000, 0x0837ff).rw(m_vdp, FUNC(gp9001vdp_device::bootleg_spriteram16_r), FUNC(gp9001vdp_device::bootleg_spriteram16_w));   // SpriteRAM
	map(0x083800, 0x087fff).ram();             // SpriteRAM (unused)
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x120000, 0x120fff).ram();             // Copy of SpriteRAM ?
//  map(0x13f000, 0x13f001).nopw();        // ???
	map(0x180000, 0x182fff).rw(m_vdp, FUNC(gp9001vdp_device::bootleg_videoram16_r), FUNC(gp9001vdp_device::bootleg_videoram16_w)); // TileRAM
	map(0x188000, 0x18800f).w(m_vdp, FUNC(gp9001vdp_device::bootleg_scroll_w));
	map(0x190003, 0x190003).r(FUNC(pipibibi_bootleg_state::shared_ram_r));  // Z80 ready ?
	map(0x190011, 0x190011).w(FUNC(pipibibi_bootleg_state::shared_ram_w)); // Z80 task to perform
	map(0x19c01d, 0x19c01d).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
	map(0x19c020, 0x19c021).portr("DSWA");
	map(0x19c024, 0x19c025).portr("DSWB");
	map(0x19c028, 0x19c029).portr("JMPR");
	map(0x19c02c, 0x19c02d).portr("SYS");
	map(0x19c030, 0x19c031).portr("IN1");
	map(0x19c034, 0x19c035).portr("IN2");
}


void pipibibi_bootleg_state::cpu_space_pipibibsbl_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff9, 0xfffff9).lr8(NAME([this] () { m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE); return m68000_device::autovector(4); }));
}

void pipibibi_state::pipibibs_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
}

void pipibibi_state::pipibibs(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10_MHz_XTAL);         // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &pipibibi_state::pipibibs_68k_mem);
	m_maincpu->reset_cb().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	Z80(config, m_audiocpu, 27_MHz_XTAL/8);         // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &pipibibi_state::pipibibs_sound_z80_mem);

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(pipibibi_state::screen_update));
	m_screen->screen_vblank().set(FUNC(pipibibi_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 27_MHz_XTAL/8)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


void pipibibi_bootleg_state::pipibibsbl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL); // ??? (position labeled "68000-12" but 10 MHz-rated parts used)
	m_maincpu->set_addrmap(AS_PROGRAM, &pipibibi_bootleg_state::pipibibi_bootleg_68k_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &pipibibi_bootleg_state::cpu_space_pipibibsbl_map);
	m_maincpu->reset_cb().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	Z80(config, m_audiocpu, 12_MHz_XTAL / 2); // GoldStar Z8400B; clock source and divider unknown
	m_audiocpu->set_addrmap(AS_PROGRAM, &pipibibi_bootleg_state::pipibibs_sound_z80_mem);

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(28.322_MHz_XTAL / 4, 450, 0, 320, 262, 0, 240); // guess, but this is within NTSC parameters
	m_screen->set_screen_update(FUNC(pipibibi_bootleg_state::screen_update));
	m_screen->screen_vblank().set(FUNC(pipibibi_bootleg_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL); // FIXME: bootleg has no VDP
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);
	m_vdp->set_bootleg_extra_offsets(0x01f, 0x1ef, 0x01d, 0x1ef, 0x01b, 0x1ef, 0x1d4, 0x1f7);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 28.322_MHz_XTAL / 8)); // ???
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void pipibibi_bootleg_state::init_pipibibsbl()
{
	u16 *ROM = (u16 *)(memregion("maincpu")->base());

	for (int i = 0; i < (0x040000/2); i += 4)
	{
		ROM[i+0] = bitswap<16>(ROM[i+0],0x1,0x5,0x6,0x7,0x8,0x2,0x0,0x9,0xe,0xd,0x4,0x3,0xf,0xa,0xb,0xc);
		ROM[i+1] = bitswap<16>(ROM[i+1],0x5,0x3,0x1,0xf,0xd,0xb,0x9,0x0,0x2,0x4,0x6,0x8,0xa,0xc,0xe,0x7);
		ROM[i+2] = bitswap<16>(ROM[i+2],0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4);
		ROM[i+3] = bitswap<16>(ROM[i+3],0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4);
	}
}

ROM_START( pipibibs )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp025-1.bin", 0x000000, 0x020000, CRC(b2ea8659) SHA1(400431b656dbfbd5a9bc5961c3ea04c4d38b6f77) )
	ROM_LOAD16_BYTE( "tp025-2.bin", 0x000001, 0x020000, CRC(dc53b939) SHA1(e4de371f97ba7c350273ad43b7f58ff31672a269) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "tp025-5.bin", 0x0000, 0x8000, CRC(bf8ffde5) SHA1(79c09cc9a0ea979f5af5a7e5ad671ea486f5f43e) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( pipibibsa )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp025-1.alt.bin", 0x000000, 0x020000, CRC(3e522d98) SHA1(043dd76b99e130909e47063d4cc773177a2eaccf) )
	ROM_LOAD16_BYTE( "tp025-2.alt.bin", 0x000001, 0x020000, CRC(48370485) SHA1(9895e086c9a5eeec4f454cbc6098adb2f66d4e11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "tp025-5.bin", 0x0000, 0x8000, CRC(bf8ffde5) SHA1(79c09cc9a0ea979f5af5a7e5ad671ea486f5f43e) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( whoopee )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "whoopee.1", 0x000000, 0x020000, CRC(28882e7e) SHA1(8fcd278a7d005eb81cd9e461139c0c0f756a4fa4) )
	ROM_LOAD16_BYTE( "whoopee.2", 0x000001, 0x020000, CRC(6796f133) SHA1(d4e657be260ba3fd3f0556ade617882513b52685) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.025", 0x00000, 0x08000, CRC(c02436f6) SHA1(385343f88991646ec23b385eaea82718f1251ea6) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( pipibibsp )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "pip_cpu_e", 0x000000, 0x020000, CRC(ae3205bd) SHA1(1613fec637dfed213433dca0d267e49f4848df81) )
	ROM_LOAD16_BYTE( "pip_cpu_o", 0x000001, 0x020000, CRC(241669a9) SHA1(234e0bb819453e16625d15d2cf22496bbc547943) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "pip_snd", 0x0000, 0x8000, CRC(8ebf183b) SHA1(602b138c85b02d121d007f6788b322aa107c7d91) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "cg_01_l", 0x000000, 0x080000, CRC(21d1ef46) SHA1(d7ccbe56eb08be421c241065cbaa99cc9cca4d73) )
	ROM_LOAD( "cg_01_h", 0x080000, 0x080000, CRC(d5726328) SHA1(26401ba8ce22fda161306b91d70afefa959cde8c) )
	ROM_LOAD( "cg_23_l", 0x100000, 0x080000, CRC(114d41d0) SHA1(d1166d495d92c6082fffbed422deb7605c5a41a2) )
	ROM_LOAD( "cg_23_h", 0x180000, 0x080000, CRC(e0468152) SHA1(f5a872d8658e959ec6cce51c7798291b5b973f15) )
ROM_END


// TODO: this runs on oneshot.cpp hardware. Move to that driver and remove the hacks in video/gp9001.cpp needed to run it in this driver
ROM_START( pipibibsbl ) /* Based off the proto code. */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ppbb06.bin", 0x000000, 0x020000, CRC(14c92515) SHA1(2d7f7c89272bb2a8115f163ad651bef3bca5107e) )
	ROM_LOAD16_BYTE( "ppbb05.bin", 0x000001, 0x020000, CRC(3d51133c) SHA1(d7bd94ad11e9aeb5a5165c5ac6f71950849bcd2f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ppbb08.bin", 0x0000, 0x8000, CRC(101c0358) SHA1(162e02d00b7bdcdd3b48a0cd0527b7428435ec50) ) // same data as komocomo in oneshot.cpp

	ROM_REGION( 0x200000, "gp9001", 0 )
	/* GFX data differs slightly from Toaplan boards ??? */
	ROM_LOAD16_BYTE( "ppbb01.bin", 0x000000, 0x080000, CRC(0fcae44b) SHA1(ac72bc79e3a5d0a81647c312d310d00ace017272) )
	ROM_LOAD16_BYTE( "ppbb02.bin", 0x000001, 0x080000, CRC(8bfcdf87) SHA1(4537a7d646d3014f069c6fd0be457bb32e2f18ac) )
	ROM_LOAD16_BYTE( "ppbb03.bin", 0x100000, 0x080000, CRC(abdd2b8b) SHA1(a4246dd63515f01d1227c9a9e16d9f1c739ee39e) )
	ROM_LOAD16_BYTE( "ppbb04.bin", 0x100001, 0x080000, CRC(70faa734) SHA1(4448f4dbded56c142e57293d371e0a422c3a667e) )

	ROM_REGION( 0x8000, "user1", 0 )            /* ??? Some sort of table */
	ROM_LOAD( "ppbb07.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) ) // 1xxxxxxxxxxxxxx = 0xFF, same data as komocomo in oneshot.cpp
ROM_END


// TODO: determine if this is the correct driver or if this needs to be moved somewhere else, too
ROM_START( pipibibsbl2 ) // PIPI001 PCB
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "06.bin", 0x000000, 0x020000, CRC(25f49c2f) SHA1(a61246ec8a07ba14ee0a01c3458c59840b435c0b) )
	ROM_LOAD16_BYTE( "07.bin", 0x000001, 0x020000, CRC(15250177) SHA1(a5ee5ccc219f300d7387b45dc8f8b72fd0f37d7e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "08.bin", 0x00000, 0x10000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) ) //  // 1ST AND 2ND HALF IDENTICAL, same as komocomo in oneshot.cpp

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD16_BYTE( "01.bin", 0x000000, 0x80000, CRC(505e9e9f) SHA1(998995d94585d785263cc926f68632065aa6c366) )
	ROM_LOAD16_BYTE( "02.bin", 0x000001, 0x80000, CRC(860018f5) SHA1(7f42dffb27940629447d688e1771b4ecf04f3b43) )
	ROM_LOAD16_BYTE( "03.bin", 0x100000, 0x80000, CRC(ece1bc0f) SHA1(d29f1520f1a3a9d276d36af650bc0d70bcb5b8da) )
	ROM_LOAD16_BYTE( "04.bin", 0x100001, 0x80000, CRC(f328d7a3) SHA1(2c4fb5d6202f847aaf7c7be719c0c92b8bb5946b) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x20000, CRC(8107c4bd) SHA1(64e2fafa808c16c722454b611a8492a4620a925c) ) // motherboard ROM, unknown purpose
ROM_END

ROM_START( pipibibsbl3 )
	ROM_REGION( 0x040000, "maincpu", 0 )            // Main 68K code, not scrambled
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x020000, CRC(7fab770c) SHA1(c96808870c5906e0203f38114702bd660e491a7d) )
	ROM_LOAD16_BYTE( "6.bin", 0x000001, 0x020000, CRC(9007ef00) SHA1(594052be7351e0b8e30f83abd9a91ab1429d82ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            // Sound Z80 code
	ROM_LOAD( "7.bin", 0x0000, 0x8000, CRC(101c0358) SHA1(162e02d00b7bdcdd3b48a0cd0527b7428435ec50) ) // same data as komocomo in oneshot.cpp

	ROM_REGION( 0x200000, "gp9001", 0 )
	// GFX data differs slightly from Toaplan boards ???
	ROM_LOAD16_BYTE( "4.bin", 0x000000, 0x080000, CRC(0fcae44b) SHA1(ac72bc79e3a5d0a81647c312d310d00ace017272) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x080000, CRC(8bfcdf87) SHA1(4537a7d646d3014f069c6fd0be457bb32e2f18ac) )
	ROM_LOAD16_BYTE( "2.bin", 0x100000, 0x080000, CRC(abdd2b8b) SHA1(a4246dd63515f01d1227c9a9e16d9f1c739ee39e) )
	ROM_LOAD16_BYTE( "1.bin", 0x100001, 0x080000, CRC(70faa734) SHA1(4448f4dbded56c142e57293d371e0a422c3a667e) )

	ROM_REGION( 0x8000, "user1", 0 )            // ??? Some sort of table
	ROM_LOAD( "8.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) ) // 1xxxxxxxxxxxxxx = 0xFF, same data as komocomo in oneshot.cpp
ROM_END

} // anonymous namespace

GAME( 1991, pipibibs,    0,        pipibibs,     pipibibs,   pipibibi_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pipibibsa,   pipibibs, pipibibs,     pipibibs,   pipibibi_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pipibibsp,   pipibibs, pipibibs,     pipibibsp,  pipibibi_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (prototype)",            MACHINE_SUPPORTS_SAVE )

GAME( 1991, pipibibsbl,  pipibibs, pipibibsbl,   pipibibsbl, pipibibi_bootleg_state, init_pipibibsbl, ROT0, "bootleg (Ryouta Kikaku)", "Pipi & Bibis / Whoopee!! (Ryouta Kikaku bootleg, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pipibibsbl2, pipibibs, pipibibsbl,   pipibibsbl, pipibibi_bootleg_state, empty_init,    ROT0,   "bootleg",                 "Pipi & Bibis / Whoopee!! (bootleg, decrypted)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // different memory map, not scrambled
GAME( 1991, pipibibsbl3, pipibibs, pipibibsbl,   pipibibsbl, pipibibi_bootleg_state, empty_init,    ROT0,   "bootleg (Ryouta Kikaku)", "Pipi & Bibis / Whoopee!! (Ryouta Kikaku bootleg, decrypted)", MACHINE_SUPPORTS_SAVE )
