// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z180/hd647180x.h"
#include "machine/gen_latch.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*

Name        Board No      Maker         Game name
----------------------------------------------------------------------------
tekipaki    TP-020        Toaplan       Teki Paki
tekipakit   TP-020        Toaplan       Teki Paki (location test)
whoopee    *TP-025/TP-020 Toaplan       Pipi & Bibis / Whoopee!! (Teki Paki hardware)

    * This version of Whoopee!! is on a board labeled TP-020
      (same board number, and same hardware, as Teki Paki)
      but the ROMs are labeled TP-025.

To Do / Unknowns:
    - Whoopee/Teki Paki sometimes tests bit 5 of the region jumper port
        just after testing for vblank. Why?
*/

namespace {

class tekipaki_state : public driver_device
{
public:
	tekipaki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
	{ }

	void tekipaki(machine_config &config);

	int c2map_r();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	u8 tekipaki_cmdavailable_r();

	void tekipaki_68k_mem(address_map &map) ATTR_COLD;
	void hd647180_io_map(address_map &map) ATTR_COLD;

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	bitmap_ind8 m_custom_priority_bitmap;
};


void tekipaki_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}


u32 tekipaki_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void tekipaki_state::screen_vblank(int state)
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


static INPUT_PORTS_START( tekipaki )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000f - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x0f, 0x02, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	// "Stop Mode" corresponds to "Invulnerability" in the other games
	// (i.e. it enables pause and slow motion)
	PORT_DIPNAME( 0x0004,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040,   0x0000, "Stop Mode (Cheat)" )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x000f,  0x0002, DEF_STR( Region ) ) PORT_DIPLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0006, "Taiwan (Spacy Co., Ltd." )
	PORT_CONFSETTING(       0x0007, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x0008, "Hong Kong (Honest Trading Co.)" )
//  PORT_CONFSETTING(        0x0009, DEF_STR( Japan ) )  // English title screen
//  PORT_CONFSETTING(        0x000a, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000b, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000c, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000d, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000e, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x000f, "Japan (Distributed by Tecmo)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(tekipaki_state::c2map_r))
INPUT_PORTS_END

static INPUT_PORTS_START( whoopee )
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
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(tekipaki_state::c2map_r))   // bit 0x10 sound ready
INPUT_PORTS_END

int tekipaki_state::c2map_r()
{
	// bit 4 high signifies secondary CPU is ready
	// bit 5 is tested low before V-Blank bit ???
	return m_soundlatch->pending_r() ? 0x00 : 0x01;
}

u8 tekipaki_state::tekipaki_cmdavailable_r()
{
	if (m_soundlatch->pending_r()) return 0xff;
	else return 0x00;
};


void tekipaki_state::tekipaki_68k_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x020000, 0x03ffff).rom();                     // extra for Whoopee
	map(0x080000, 0x082fff).ram();
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x14000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x180000, 0x180001).portr("DSWA");
	map(0x180010, 0x180011).portr("DSWB");
	map(0x180020, 0x180021).portr("SYS");
	map(0x180030, 0x180031).portr("JMPR");           // CPU 2 busy and Region Jumper block
	map(0x180041, 0x180041).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
	map(0x180050, 0x180051).portr("IN1");
	map(0x180060, 0x180061).portr("IN2");
	map(0x180071, 0x180071).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void tekipaki_state::hd647180_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x60, 0x60).nopr();
	map(0x70, 0x75).nopw(); // DDRs are written with the wrong upper addresses!
	map(0x84, 0x84).r(m_soundlatch, FUNC(generic_latch_8_device::read));

	map(0x82, 0x82).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x83, 0x83).w("ymsnd", FUNC(ym3812_device::data_w));
}

void tekipaki_state::tekipaki(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10_MHz_XTAL);         // 10MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &tekipaki_state::tekipaki_68k_mem);
	m_maincpu->reset_cb().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	hd647180x_device &audiocpu(HD647180X(config, m_audiocpu, 10_MHz_XTAL));
	// 16k byte ROM and 512 byte RAM are internal
	audiocpu.set_addrmap(AS_IO, &tekipaki_state::hd647180_io_map);
	audiocpu.in_pa_callback().set(FUNC(tekipaki_state::tekipaki_cmdavailable_r));

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(tekipaki_state::screen_update));
	m_screen->screen_vblank().set(FUNC(tekipaki_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 27_MHz_XTAL/8));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( tekipaki )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp020-1.bin", 0x000000, 0x010000, CRC(d8420bd5) SHA1(30c1ad9e053cd7e79adb42aa428ebee28e144755) )
	ROM_LOAD16_BYTE( "tp020-2.bin", 0x000001, 0x010000, CRC(7222de8e) SHA1(8352ae23efc24a2e20cc24b6d37cb8fc6b1a730c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	ROM_LOAD( "hd647180.020", 0x00000, 0x08000, CRC(d5157c12) SHA1(b2c6c087bb539456a9e562d0b40f05dde26cacd3) )

	ROM_REGION( 0x100000, "gp9001", 0 )
	ROM_LOAD( "tp020-4.bin", 0x000000, 0x080000, CRC(3ebbe41e) SHA1(cea196c5f83e1a23d5b538a0db9bbbffa7af5118) )
	ROM_LOAD( "tp020-3.bin", 0x080000, 0x080000, CRC(2d5e2201) SHA1(5846c844eedd48305c1c67dc645b6e070b3f5b98) )
ROM_END


ROM_START( tekipakit ) /* Location Test version */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "e.e5", 0x000000, 0x010000, CRC(89affc73) SHA1(3930bf0c2528de28dcb0cf2cd537adb62a2172e3) ) /* hand written "E"  27C512 chip */
	ROM_LOAD16_BYTE( "o.e6", 0x000001, 0x010000, CRC(a2244558) SHA1(5291cfbea4d4d1c45d6d4bd21b3c466459a0fa17) ) /* hand written "O"  27C512 chip */

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	ROM_LOAD( "hd647180.020", 0x00000, 0x08000, CRC(d5157c12) SHA1(b2c6c087bb539456a9e562d0b40f05dde26cacd3) )

	ROM_REGION( 0x100000, "gp9001", 0 )
	ROM_LOAD( "0-1_4.4_cb45.a16", 0x000000, 0x080000, CRC(35e14729) SHA1(8c929604953b78c6e72744a38e06a988510193a5) ) /* hand written "0-1  4/4  CB45"  27C402 chip */
	ROM_LOAD( "3-4_4.4_547d.a15", 0x080000, 0x080000, CRC(41975fcc) SHA1(f850d5a9638d41bb69f204a9cd54e2fd693b57ef) ) /* hand written "3-4  4/4  547D"  27C402 chip */
ROM_END

ROM_START( whoopee )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "whoopee.1", 0x000000, 0x020000, CRC(28882e7e) SHA1(8fcd278a7d005eb81cd9e461139c0c0f756a4fa4) )
	ROM_LOAD16_BYTE( "whoopee.2", 0x000001, 0x020000, CRC(6796f133) SHA1(d4e657be260ba3fd3f0556ade617882513b52685) )

	ROM_REGION( 0x8000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.025", 0x00000, 0x08000, CRC(c02436f6) SHA1(385343f88991646ec23b385eaea82718f1251ea6) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END

} // anonymous namespace

GAME( 1991, tekipaki,    0,        tekipaki,     tekipaki,   tekipaki_state, empty_init,    ROT0,   "Toaplan",         "Teki Paki",                 MACHINE_SUPPORTS_SAVE )
GAME( 1991, tekipakit,   tekipaki, tekipaki,     tekipaki,   tekipaki_state, empty_init,    ROT0,   "Toaplan",         "Teki Paki (location test)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, whoopee,     pipibibs, tekipaki,     whoopee,    tekipaki_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (Teki Paki hardware)",   MACHINE_SUPPORTS_SAVE ) // original Whoopee!! boards have a HD647180 instead of Z80
