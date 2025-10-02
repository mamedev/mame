// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplan_v25_tables.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/v25.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

/*
Name        Board No      Maker         Game name
----------------------------------------------------------------------------
grindstm    TP-027        Toaplan       Grind Stormer (1992)
grindstma   TP-027        Toaplan       Grind Stormer (1992) (older)
vfive       TP-027        Toaplan       V-V (V-Five)  (1993 - Japan only)


grindstm - Code at 20A26 in vfive forces region to Japan. All sets have some NOPs at reset vector,
            and the NEC V25 CPU test that the other games do is skipped. Furthermore, all sets have
            a broken ROM checksum routine that reads address ranges that don't match the actual
            location or size of the ROM, and that has a hack at the end so it always passes.
            Normally you would expect to see code like this in a bootleg, but the NOPs and other
            oddities are identical among three different sets.
*/

namespace {

class vfive_state : public driver_device
{
public:
	vfive_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_shared_ram(*this, "shared_ram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_coincounter(*this, "coincounter")
	{ }
	void vfive(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void vfive_68k_mem(address_map &map) ATTR_COLD;
	void vfive_v25_mem(address_map &map) ATTR_COLD;

	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }
	void coin_sound_reset_w(u8 data);
	void reset_audiocpu(int state);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	required_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	required_device<m68000_base_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<toaplan_coincounter_device> m_coincounter;
	bitmap_ind8 m_custom_priority_bitmap;
};


void vfive_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}

u32 vfive_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void vfive_state::screen_vblank(int state)
{
	if (state) // rising edge
	{
		m_vdp->screen_eof();
	}
}

void vfive_state::reset_audiocpu(int state)
{
	if (state)
		coin_sound_reset_w(0);
}

void vfive_state::machine_reset()
{
	coin_sound_reset_w(0);
}

void vfive_state::coin_sound_reset_w(u8 data)
{
	m_coincounter->coin_w(data & ~0x10);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
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

static INPUT_PORTS_START( grindstm )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Cabinet ) )        PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(        0x0001, DEF_STR( Cocktail ) )
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0xe0, 0x80, SW1 )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "300k and 800k" )
	PORT_DIPSETTING(        0x0004, "300k and every 800k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	// Code in many places in game tests if region is >= 0xC. Effects on gameplay?
	PORT_CONFNAME( 0x00f0,  0x0090, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0090, DEF_STR( Europe ) )
//  PORT_CONFSETTING(        0x0080, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00b0, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, "USA (American Sammy Corporation)" )
	PORT_CONFSETTING(       0x0070, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0040, "Taiwan (Anomoto International Inc.)" )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0020, "Hong Kong (Charterfield)" )
	PORT_CONFSETTING(       0x0010, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x00d0, "USA; different?" )
	PORT_CONFSETTING(       0x00c0, "USA (American Sammy Corporation); different?" )
	PORT_CONFSETTING(       0x00e0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00f0, "Korea; different?" )
INPUT_PORTS_END

static INPUT_PORTS_START( grindstma )
	PORT_INCLUDE( grindstm )

	PORT_MODIFY("JMPR")
	// Code in many places in game tests if region is >= 0xC. Effects on gameplay?
	PORT_CONFNAME( 0x00f0,  0x0090, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0090, DEF_STR( Europe ) )
//  PORT_CONFSETTING(        0x0080, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00b0, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, "USA (Atari Games Corp.)" )
	PORT_CONFSETTING(       0x0070, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
//  PORT_CONFSETTING(        0x0040, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0020, "Hong Kong (Charterfield)" )
	PORT_CONFSETTING(       0x0010, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x00c0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00d0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00e0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00f0, "Korea; different?" )
INPUT_PORTS_END

static INPUT_PORTS_START( vfive )
	PORT_INCLUDE( grindstm )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_JAPAN_LOC(SW1)

	PORT_MODIFY("JMPR")
	// Region is forced to Japan in this set.
	// Code at $9238 tests bit 7.
	// (Actually bit 3, but the V25 shifts the jumper byte before storing it in shared RAM)
	// Runs twice near end of stage 1, once when each of the two boss tanks appears. Effect?
	// Also, if bit 7 is set and bits 6-5 are clear, service mode wrongly shows European coinage
	// (due to code left in from Grind Stormer: see code at $210A4 and lookup table at $211FA)
	PORT_CONFNAME( 0x0030,  0x0000, "Copyright" )           //PORT_CONFLOCATION("JP:!4,!3")
	PORT_CONFSETTING(       0x0000, "All Rights Reserved" )
//  PORT_CONFSETTING(        0x0010, "All Rights Reserved" )
//  PORT_CONFSETTING(        0x0020, "All Rights Reserved" )
	PORT_CONFSETTING(       0x0030, "Licensed to Taito Corp." )
	PORT_CONFNAME( 0x0040,  0x0000, DEF_STR( Unused ) )     //PORT_CONFLOCATION("JP:!2")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( On ) )
	PORT_CONFNAME( 0x0080,  0x0000, DEF_STR( Unknown ) )    //PORT_CONFLOCATION("JP:!1")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0080, DEF_STR( On ) )
INPUT_PORTS_END

void vfive_state::vfive_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
//  map(0x200000, 0x20ffff).noprw(); // Read at startup by broken ROM checksum code - see notes
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001d, 0x20001d).w(FUNC(vfive_state::coin_sound_reset_w)); // Coin count/lock + v25 reset line
	map(0x210000, 0x21ffff).rw(FUNC(vfive_state::shared_ram_r), FUNC(vfive_state::shared_ram_w)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x700001).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
}


void vfive_state::vfive_v25_mem(address_map &map)
{
	map(0x00000, 0x00001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).ram().share(m_shared_ram);
}


void vfive_state::vfive(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &vfive_state::vfive_68k_mem);
	m_maincpu->reset_cb().set(FUNC(vfive_state::reset_audiocpu));

	v25_device &audiocpu(V25(config, m_audiocpu, 20_MHz_XTAL/2)); // Verified on PCB, NEC V25 type Toaplan mark scratched out
	audiocpu.set_addrmap(AS_PROGRAM, &vfive_state::vfive_v25_mem);
	audiocpu.set_decryption_table(toaplan_v25_tables::nitro_decryption_table);
	audiocpu.pt_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	TOAPLAN_COINCOUNTER(config, m_coincounter, 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240); // verified on PCB
	m_screen->set_screen_update(FUNC(vfive_state::screen_update));
	m_screen->screen_vblank().set(FUNC(vfive_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5); // verified on PCB
}



ROM_START( grindstm )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "01.bin", 0x000000, 0x080000, CRC(4923f790) SHA1(1c2d66b432d190d0fb6ac7ca0ec0687aea3ccbf4) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( grindstma )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp027-01.rom", 0x000000, 0x080000, CRC(8d8c0392) SHA1(824dde274c8bef8a87c54d8ccdda7f0feb8d11e1) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( vfive )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp027_01.bin", 0x000000, 0x080000, CRC(731d50f4) SHA1(794255d0a809cda9170f5bac473df9d7f0efdac8) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END

} // anonymous namespace

GAME( 1992, grindstm,    0,        vfive,      grindstm,   vfive_state, empty_init,      ROT270, "Toaplan", "Grind Stormer",             MACHINE_SUPPORTS_SAVE )
GAME( 1992, grindstma,   grindstm, vfive,      grindstma,  vfive_state, empty_init,      ROT270, "Toaplan", "Grind Stormer (older set)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, vfive,       grindstm, vfive,      vfive,      vfive_state, empty_init,      ROT270, "Toaplan", "V-Five (Japan)",            MACHINE_SUPPORTS_SAVE )
