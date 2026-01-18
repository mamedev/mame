// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*
Name        Board No      Maker         Game name
----------------------------------------------------------------------------
snowbro2    TP-033        Hanafram      Snow Bros. 2 - With New Elves

*/

namespace {

class snowbro2_state : public driver_device
{
public:
	snowbro2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdp(*this, "gp9001")
		, m_oki(*this, "oki")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void snowbro2(machine_config &config) ATTR_COLD;
	void snowbro2b3(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	void snowbro2_68k_mem(address_map &map) ATTR_COLD;
	void snowbro2b3_68k_mem(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void sb2_oki_bankswitch_w(u8 data);

private:
	required_device<m68000_base_device> m_maincpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	bitmap_ind8 m_custom_priority_bitmap;
};


void snowbro2_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}

u32 snowbro2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void snowbro2_state::screen_vblank(int state)
{
	if (state) // rising edge
	{
		m_vdp->screen_eof();
	}
}

void snowbro2_state::sb2_oki_bankswitch_w(u8 data)
{
	m_oki->set_rom_bank(data & 1);
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

static INPUT_PORTS_START( snowbro2 )
	PORT_INCLUDE( base )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0001, "Discount" )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x1c00, 0x0800, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and every 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )     PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, "Maximum Players" )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "2" )
	PORT_DIPSETTING(        0x0000, "4" )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x2000,  0x0000, "Show All Rights Reserved" )    //PORT_CONFLOCATION("JP:!1")
	PORT_CONFSETTING(       0x0000, DEF_STR( No ) )
	PORT_CONFSETTING(       0x2000, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x1c00,  0x0800, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2")
	PORT_CONFSETTING(       0x0800, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0400, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0c00, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x1000, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x1400, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x1800, DEF_STR( Southeast_Asia ) )
//  PORT_CONFSETTING(        0x1c00, DEF_STR( Unused ) )
	PORT_BIT( 0xc3ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( snowbro2b3 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x0400, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Discount" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW1:!3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and every 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )     PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, "Maximum Players" )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "2" )
	PORT_DIPSETTING(        0x0000, "4" )
INPUT_PORTS_END

void snowbro2_state::snowbro2_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700001).portr("JMPR");
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700010, 0x700011).portr("IN2");
	map(0x700014, 0x700015).portr("IN3");
	map(0x700018, 0x700019).portr("IN4");
	map(0x70001c, 0x70001d).portr("SYS");
	map(0x700031, 0x700031).w(FUNC(snowbro2_state::sb2_oki_bankswitch_w));
	map(0x700035, 0x700035).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
}

void snowbro2_state::snowbro2b3_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x404000, 0x404fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700010, 0x700011).portr("IN2");
	map(0x700014, 0x700015).portr("IN3");
	map(0x700018, 0x700019).portr("IN4");
	map(0x700035, 0x700035).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
	map(0x700041, 0x700041).w(FUNC(snowbro2_state::sb2_oki_bankswitch_w));
	map(0xff0000, 0xff2fff).rw(m_vdp, FUNC(gp9001vdp_device::bootleg_videoram16_r), FUNC(gp9001vdp_device::bootleg_videoram16_w));
	map(0xff3000, 0xff37ff).rw(m_vdp, FUNC(gp9001vdp_device::bootleg_spriteram16_r), FUNC(gp9001vdp_device::bootleg_spriteram16_w));
	map(0xff8000, 0xff800f).w(m_vdp, FUNC(gp9001vdp_device::bootleg_scroll_w));
}

void snowbro2_state::snowbro2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &snowbro2_state::snowbro2_68k_mem);

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(snowbro2_state::screen_update));
	m_screen->screen_vblank().set(FUNC(snowbro2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki, 16_MHz_XTAL/4, okim6295_device::PIN7_LOW);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void snowbro2_state::snowbro2b3(machine_config &config)
{
	snowbro2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &snowbro2_state::snowbro2b3_68k_mem);
	m_maincpu->set_vblank_int("screen", FUNC(snowbro2_state::irq2_line_hold));

	m_vdp->vint_out_cb().set_nop();
	m_vdp->set_bootleg_extra_offsets(0x02e, 0x1f0, 0x02e, 0x1ee, 0x02e, 0x1ef, 0x1e9, 0x1ef);
}


ROM_START( snowbro2 )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "pro-4", 0x000000, 0x080000, CRC(4c7ee341) SHA1(ad46c605a38565d0148daac301be4e4b72302fe7) )

	ROM_REGION( 0x300000, "gp9001", 0 )
	ROM_LOAD( "rom2-l", 0x000000, 0x100000, CRC(e9d366a9) SHA1(e87e3966fce3395324b90db6c134b3345104c04b) )
	ROM_LOAD( "rom2-h", 0x100000, 0x080000, CRC(9aab7a62) SHA1(611f6a15fdbac5d3063426a365538c1482e996bf) )
	ROM_LOAD( "rom3-l", 0x180000, 0x100000, CRC(eb06e332) SHA1(7cd597bfffc153d178530c0f0903bebd751c9dd1) )
	ROM_LOAD( "rom3-h", 0x280000, 0x080000, CRC(df4a952a) SHA1(b76af61c8437caca573ff1312832898666a611aa) )

	ROM_REGION( 0x80000, "oki", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom4", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END

ROM_START( snowbro2b ) // seems to be the same data as the main set, but with the extra user1 rom and different rom layout
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code - difference with main set is year changed from 1994 to 1998 and upper FFFF fill changed to 00FF fill */
	ROM_LOAD16_BYTE( "sb2-prg1.u39", 0x000000, 0x040000, CRC(e1fec8a2) SHA1(30c1a351070d784da9ba0dca68be8a262dba2045) )
	ROM_LOAD16_BYTE( "sb2-prg0.u23", 0x000001, 0x040000, CRC(b473cd57) SHA1(331130faa9de01b3ca93845174e8c3684bd269c7) )

	ROM_REGION( 0x400000, "gp9001", 0 )
	ROM_LOAD( "sb2-gfx.u177", 0x000000, 0x200000, CRC(ebeec910) SHA1(e179f393b98135caa8419b68cd979038ab47a413) )
	ROM_LOAD( "sb2-gfx.u175", 0x200000, 0x200000, CRC(e349c75b) SHA1(7d40d00fc0e15a68c427fe94db410bb7cbe00117) )

	ROM_REGION( 0x80000, "oki", 0 )         /* ADPCM Samples */
	ROM_LOAD( "sb2-snd-4.u17", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )

	ROM_REGION( 0x8000, "user1", 0 )        /* ??? Some sort of table - same as other bootleg boards */
	ROM_LOAD( "sb2-unk.u100", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )
ROM_END

ROM_START( snowbro2b2 ) // seems to mostly be the same data, but with copyright changed to Q Elec. Only set with staff credits still present. Also differently arranged graphics ROMs data.
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "rom10.bin", 0x000000, 0x080000, CRC(3e96da41) SHA1(692211d40f506efb9cb49848521de2da7890e248) ) // 27c04002

	ROM_REGION( 0x300000, "gp9001", 0 )
	ROM_LOAD16_BYTE( "rom07.bin", 0x000000, 0x080000, CRC(c54ae0b3) SHA1(94099b2da52eb12638799eab0819fe8a13aa3879) ) // 27c040
	ROM_LOAD16_BYTE( "rom05.bin", 0x000001, 0x080000, CRC(af3c74d1) SHA1(e97a688db50dfe41723452a9f652564e89e367ed) ) // 27c040
	ROM_LOAD16_BYTE( "rom08.bin", 0x100000, 0x040000, CRC(72812088) SHA1(1c0d410a7dd8de0bc48b7ff677979ad269966f7d) ) // 27c02001
	ROM_LOAD16_BYTE( "rom06.bin", 0x100001, 0x040000, CRC(c8f80774) SHA1(004752d7dfa08c3beb774f545fe3260d328abff0) ) // 27c02001
	ROM_LOAD16_BYTE( "rom03.bin", 0x180000, 0x080000, CRC(42fecbd7) SHA1(96dc9d5495d7830400ca7475c6613119099e93f2) ) // 27c040
	ROM_LOAD16_BYTE( "rom01.bin", 0x180001, 0x080000, CRC(e7134937) SHA1(7c12e7c6b08f804613e5ea0db8d622bda01bc036) ) // 27c040
	ROM_LOAD16_BYTE( "rom04.bin", 0x280000, 0x040000, CRC(3343b7a7) SHA1(10efcb2dfae635f005773655faa573bf51ddc6a3) ) // 27c020
	ROM_LOAD16_BYTE( "rom02.bin", 0x280001, 0x040000, CRC(af4d9551) SHA1(adcf1641e37b239b1ae4322b5710d49e53c30684) ) // 27c020

	ROM_REGION( 0x80000, "oki", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom09.bin", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END

ROM_START( snowbro2b3 ) // SK000616 PCB, no original parts, seems hardcoded on Europe region
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "prg", 0x000000, 0x080000, CRC(8ce2ede2) SHA1(ddd8a2aa442cd5bb3a7d393b9b5c06fd981e7c61) )

	ROM_REGION( 0x400000, "gp9001", 0 ) // not actually a GP9001
	ROM_LOAD( "gfx2", 0x100000, 0x100000, CRC(a3be41af) SHA1(4cb1ce9c47bf8bbf7d1e36f6a1d276ce52957cfb) )
	ROM_CONTINUE(     0x000000, 0x100000 )
	ROM_LOAD( "gfx1", 0x300000, 0x100000, CRC(8df1ab06) SHA1(2a28caf7d545dc05acfcd2a8d2ffbd9f710af45d) )
	ROM_CONTINUE(     0x200000, 0x100000 )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "voice", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END

ROM_START( snowbro2ny ) // Nyanko
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "rom1_c8.u61", 0x000000, 0x080000, CRC(9e6eb76b) SHA1(9e8b356dabedeb4ae9e08d60fbf6ed4a09edc0bd) )

	ROM_REGION( 0x300000, "gp9001", 0 )
	ROM_LOAD( "rom2-l_tp-033.u13", 0x000000, 0x100000, CRC(e9d366a9) SHA1(e87e3966fce3395324b90db6c134b3345104c04b) )
	ROM_LOAD( "rom2-h_c10.u26",    0x100000, 0x080000, CRC(9aab7a62) SHA1(611f6a15fdbac5d3063426a365538c1482e996bf) )
	ROM_LOAD( "rom3-l_tp-033.u12", 0x180000, 0x100000, CRC(eb06e332) SHA1(7cd597bfffc153d178530c0f0903bebd751c9dd1) )
	ROM_LOAD( "rom3-h_c9.u27",     0x280000, 0x080000, CRC(6de2b059) SHA1(695e789849c34de5d83e40b0e834b2106fcd78db) )

	ROM_REGION( 0x80000, "oki", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom4-tp-033.u33", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )

	ROM_REGION( 0x345, "plds", 0 )
	ROM_LOAD( "13_gal16v8-25lnc.u91", 0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "14_gal16v8-25lnc.u92", 0x117, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "15_gal16v8-25lnc.u93", 0x22e, 0x117, NO_DUMP ) // Protected
ROM_END

} // anonymous namespace

GAME( 1994, snowbro2,    0,        snowbro2,   snowbro2,   snowbro2_state, empty_init,      ROT0,   "Hanafram",         "Snow Bros. 2 - With New Elves / Otenki Paradise (Hanafram)",       MACHINE_SUPPORTS_SAVE )
GAME( 1994, snowbro2ny,  snowbro2, snowbro2,   snowbro2,   snowbro2_state, empty_init,      ROT0,   "Nyanko",           "Snow Bros. 2 - With New Elves / Otenki Paradise (Nyanko)",         MACHINE_SUPPORTS_SAVE ) // not a bootleg, has original parts (the "GP9001 L7A0498 TOA PLAN" IC and the three mask ROMs)
GAME( 1998, snowbro2b,   snowbro2, snowbro2,   snowbro2,   snowbro2_state, empty_init,      ROT0,   "bootleg",          "Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, snowbro2b2,  snowbro2, snowbro2,   snowbro2,   snowbro2_state, empty_init,      ROT0,   "bootleg (Q Elec)", "Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 2)", MACHINE_SUPPORTS_SAVE ) // possibly not a bootleg, has some original parts
GAME( 1994, snowbro2b3,  snowbro2, snowbro2b3, snowbro2b3, snowbro2_state, empty_init,      ROT0,   "bootleg",          "Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // GFX offsets not 100% correct
