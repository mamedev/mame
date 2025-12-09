// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplan_coincounter.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z180/hd647180x.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*
Name        Board No      Maker         Game name
----------------------------------------------------------------------------
ghox        TP-021        Toaplan       Ghox (Spinner with single up/down axis control)
ghoxj       TP-021        Toaplan       Ghox (8-Way Joystick controls)


ghox     - The ghoxj set displays an English title screen when the jumpers are set for Japan/Taito,
            and fails to display the "Winners Don't Use Drugs" logo when set for USA/Taito (either
            Taito America or Taito Japan).
*/

namespace {

class ghox_state : public driver_device
{
public:
	ghox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_io_pad(*this, "PAD%u", 1U)
		, m_shared_ram(*this, "shared_ram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void ghox(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void ghox_68k_mem(address_map &map) ATTR_COLD;
	void ghox_hd647180_mem_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	template<unsigned Which> u16 ghox_h_analog_r();
	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }

	s8 m_old_paddle_h[2] = {0};

	required_ioport_array<2> m_io_pad;
	required_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	required_device<m68000_base_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	bitmap_ind8 m_custom_priority_bitmap;
};


void ghox_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}


u32 ghox_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void ghox_state::screen_vblank(int state)
{
	if (state) // rising edge
	{
		m_vdp->screen_eof();
	}
}

void ghox_state::machine_start()
{
	save_item(NAME(m_old_paddle_h));
}

void ghox_state::machine_reset()
{
	m_old_paddle_h[0] = 0;
	m_old_paddle_h[1] = 0;
}

template<unsigned Which>
u16 ghox_state::ghox_h_analog_r()
{
	const s8 new_value = m_io_pad[Which]->read();
	const s8 result = new_value - m_old_paddle_h[Which];
	if (!machine().side_effects_disabled())
		m_old_paddle_h[Which] = new_value;
	return result;
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

static INPUT_PORTS_START( ghox )
	PORT_INCLUDE( base )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000f - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x80000, 0x80000, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	// "Debug Mode" corresponds to "Invulnerability" in the other games
	// (i.e. it enables pause and slow motion)
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and 300k" )
	PORT_DIPSETTING(        0x0000, "100k and every 200k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Debug Mode (Cheat)" )  PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )

	PORT_START("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x8000f, 0x80002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,!1,FAKE:!1")
	PORT_CONFSETTING(       0x80002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x00003, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x00004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x00005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x80007, "Italy (Star Electronica SRL)" )
	PORT_CONFSETTING(       0x80008, "UK (JP Leisure Limited)" )
	PORT_CONFSETTING(       0x00009, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x8000a, "Europe (Nova Apparate GMBH & Co.)" )
	PORT_CONFSETTING(       0x0000b, "USA (Taito America Corporation)" )
	PORT_CONFSETTING(       0x0000c, "USA (Taito Corporation Japan)" )
	PORT_CONFSETTING(       0x8000d, "Europe (Taito Corporation Japan)" )
	PORT_CONFSETTING(        0x0000e, "Japan (Licensed to [blank])" )    // English title screen
	PORT_CONFSETTING(        0x0000f, "Japan (Taito Corporation)" )

	PORT_START("PAD1")      /* Paddle 1 (left-right)  read at $100000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("PAD2")      /* Paddle 2 (left-right)  read at $040000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused
INPUT_PORTS_END

static INPUT_PORTS_START( ghoxjo )
	PORT_INCLUDE( ghox )

	PORT_MODIFY("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x8000f, 0x80002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,!1,FAKE:!1")
	PORT_CONFSETTING(       0x80002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x00003, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x00004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x00005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x80007, "Italy (Star Electronica SRL)" )
	PORT_CONFSETTING(       0x80008, "UK (JP Leisure Limited)" )
	PORT_CONFSETTING(       0x00009, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x8000a, "Europe (Nova Apparate GMBH & Co.)" )
	PORT_CONFSETTING(       0x0000b, "Japan (Unused) [b]" )
	PORT_CONFSETTING(       0x0000c, "Japan (Unused) [c]" )
	PORT_CONFSETTING(       0x0000d, "Japan (Unused) [d]" )
	PORT_CONFSETTING(       0x0000e, "Japan (Unused) [e]" )
	PORT_CONFSETTING(       0x0000f, "Japan (Unused) [f]" )
INPUT_PORTS_END


void ghox_state::ghox_68k_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x040001).r(FUNC(ghox_state::ghox_h_analog_r<1>));
	map(0x080000, 0x083fff).ram();
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x100000, 0x100001).r(FUNC(ghox_state::ghox_h_analog_r<0>));
	map(0x140000, 0x14000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x180000, 0x180fff).rw(FUNC(ghox_state::shared_ram_r), FUNC(ghox_state::shared_ram_w)).umask16(0x00ff);
	map(0x181001, 0x181001).w("coincounter", FUNC(toaplan_coincounter_device::coin_w));
	map(0x18100c, 0x18100d).portr("JMPR");
}

void ghox_state::ghox_hd647180_mem_map(address_map &map)
{
	map(0x40000, 0x407ff).ram().share(m_shared_ram);

	map(0x80002, 0x80002).portr("DSWA");
	map(0x80004, 0x80004).portr("DSWB");
	map(0x80006, 0x80006).nopr(); // nothing?
	map(0x80008, 0x80008).portr("IN1");
	map(0x8000a, 0x8000a).portr("IN2");

	map(0x8000c, 0x8000e).portr("SYS");

	map(0x8000e, 0x8000f).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}


void ghox_state::ghox(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10_MHz_XTAL);         /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &ghox_state::ghox_68k_mem);
	m_maincpu->reset_cb().set_inputline(m_audiocpu, INPUT_LINE_RESET);

	HD647180X(config, m_audiocpu, 10_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghox_state::ghox_hd647180_mem_map);

	TOAPLAN_COINCOUNTER(config, "coincounter", 0);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(ghox_state::screen_update));
	m_screen->screen_vblank().set(FUNC(ghox_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5); // verified on pcb
}


ROM_START( ghox ) /* Spinner with single axis (up/down) controls */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01.u10", 0x000000, 0x020000, CRC(9e56ac67) SHA1(daf241d9e55a6e60fc004ed61f787641595b1e62) )
	ROM_LOAD16_BYTE( "tp021-02.u11", 0x000001, 0x020000, CRC(15cac60f) SHA1(6efa3a50a5dfe6ef4072738d6a7d0d95dca8a675) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, CRC(6ab59e5b) SHA1(d814dd3a8f1ee638794e2bd422eed4247ba4a15e) )

	ROM_REGION( 0x100000, "gp9001", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END


ROM_START( ghoxj ) /* 8-way joystick for controls */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01a.u10", 0x000000, 0x020000, CRC(c11b13c8) SHA1(da7defc1d3b6ddded910ba56c31fbbdb5ed57b09) )
	ROM_LOAD16_BYTE( "tp021-02a.u11", 0x000001, 0x020000, CRC(8d426767) SHA1(1ed4a8bcbf4352257e7d58cb5c2c91eb48c2f047) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, CRC(6ab59e5b) SHA1(d814dd3a8f1ee638794e2bd422eed4247ba4a15e) )

	ROM_REGION( 0x100000, "gp9001", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END

ROM_START( ghoxjo ) /* older version (with fewer regions) of the 8-way joystick version */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01.ghoxsticker.u10", 0x000000, 0x020000, CRC(ad3a8817) SHA1(317267e0c00934a86bf05c5afd6c69a7944a2ed3) ) // TP021 ?01? label covered with a handwriten 'GHOX' sticker
	ROM_LOAD16_BYTE( "tp021-02.ghoxsticker.u11", 0x000001, 0x020000, CRC(2340e981) SHA1(d8e3f55e67fe6500f9e6c7eed1388dc895c5f574) ) // TP021 ?02? label covered with a handwriten 'GHOX' sticker

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, CRC(6ab59e5b) SHA1(d814dd3a8f1ee638794e2bd422eed4247ba4a15e) )

	ROM_REGION( 0x100000, "gp9001", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END

} // anonymous namespace

GAME( 1991, ghox,        0,        ghox,         ghox,       ghox_state,     empty_init,    ROT270, "Toaplan",         "Ghox (spinner)",            MACHINE_SUPPORTS_SAVE )
GAME( 1991, ghoxj,       ghox,     ghox,         ghox,       ghox_state,     empty_init,    ROT270, "Toaplan",         "Ghox (joystick)",           MACHINE_SUPPORTS_SAVE )
GAME( 1991, ghoxjo,      ghox,     ghox,         ghoxjo,     ghox_state,     empty_init,    ROT270, "Toaplan",         "Ghox (joystick, older)",    MACHINE_SUPPORTS_SAVE )
