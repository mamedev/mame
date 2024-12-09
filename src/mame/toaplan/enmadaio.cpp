// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "gp9001.h"
#include "toaplipt.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

/*
Name        Board No      Maker         Game name
----------------------------------------------------------------------------
enmadaio    TP-031        Toaplan       Enma Daio
*/

namespace {

class enmadaio_state : public driver_device
{
public:
	enmadaio_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdp(*this, "gp9001")
		, m_oki(*this, "oki")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_okibank(*this, "okibank")
	{ }

	void enmadaio(machine_config &config) ATTR_COLD;

	void init_enmadaio() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:

	void enmadaio_oki_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void enmadaio_68k_mem(address_map &map) ATTR_COLD;
	void enmadaio_oki(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	required_device<m68000_base_device> m_maincpu;
	required_device<gp9001vdp_device> m_vdp;
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_bank m_okibank;
	bitmap_ind8 m_custom_priority_bitmap;
};

void enmadaio_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;
}

u32 enmadaio_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);
	return 0;
}

void enmadaio_state::screen_vblank(int state)
{
	if (state) 	// rising edge
	{
		m_vdp->screen_eof();
	}
}

static INPUT_PORTS_START( enmadaio )
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
	PORT_DIPNAME( 0x0001, 0x0000, "Game Mode" )
	PORT_DIPSETTING(      0x0000, "Normal Game" )
	PORT_DIPSETTING(      0x0001, "Stop and Slow Mode" )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) // unused
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) // unused
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) // unused
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("MISC2") // should be DSWB? but not even read in test mode display?
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("MISC3")
	PORT_DIPNAME( 0x0001, 0x0001, "3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("yes")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("no")
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("MISC4")
	PORT_DIPNAME( 0x0001, 0x0001, "4" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Status of Something 1" ) // gives error in attract mode otherwise
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Status of Something 2" ) // turn this off and it comes up with an error
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("UNK")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) // this is the sensor, not sure what it measures, pulse / heartbeat?
	PORT_DIPNAME( 0x0100, 0x0100, "Freeze" ) // not sure
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


void enmadaio_state::enmadaio_oki_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	data &= mem_mask;

	if (data < 0x60)
	{
		m_okibank->set_entry(data);
	}
	else
	{
		logerror("enmadaio_oki_bank_w >=0x60 (%04x)\n",data);
	}
}

void enmadaio_state::enmadaio_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram(); //.share("nvram");
	map(0x104000, 0x10ffff).ram();

	map(0x200000, 0x20000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x300000, 0x300fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x500001, 0x500001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x700000, 0x700001).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));

	map(0x700004, 0x700005).portr("DSWA");
	map(0x70000c, 0x70000d).portr("MISC2");
	map(0x700010, 0x700011).portr("MISC3");
	map(0x700014, 0x700015).portr("MISC4");
	map(0x700018, 0x700019).portr("SYS");
	map(0x70001c, 0x70001d).portr("UNK"); //.portr("SYS");

	map(0x700020, 0x700021).w(FUNC(enmadaio_state::enmadaio_oki_bank_w)); // oki bank

	map(0x700028, 0x700029).nopw();
	map(0x70003c, 0x70003d).nopw();
	map(0x70002c, 0x70002d).nopw();
}

void enmadaio_state::enmadaio_oki(address_map &map)
{
	map(0x00000, 0x3ffff).bankr(m_okibank);
}


void enmadaio_state::enmadaio(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &enmadaio_state::enmadaio_68k_mem);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(enmadaio_state::screen_update));
	m_screen->screen_vblank().set(FUNC(enmadaio_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki, 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // pin7 not confirmed
	m_oki->set_addrmap(0, &enmadaio_state::enmadaio_oki);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void enmadaio_state::init_enmadaio()
{
	u8 *ROM = memregion("oki")->base();

	m_okibank->configure_entries(0, 0x60, ROM, 0x40000);
	m_okibank->set_entry(0);
}


ROM_START( enmadaio )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "03n_u53.c8",        0x00000, 0x80000, CRC(1a6ca2ee) SHA1(13d34a10004ca172db7953e2be8daa90fc5b62ed) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "rom4_u30.c19",       0x000000, 0x100000, CRC(7a012d8b) SHA1(a33d9490573a9fd1e799d3fe567f991926851c51) )
	ROM_LOAD( "rom5_u31.c18",       0x100000, 0x100000, CRC(60b127ab) SHA1(98785dfd6a86b4bd2c9786f6f72796c023b5b73a) )

	ROM_REGION( 0x1800000, "oki", 0 )    /* OKIM6295 samples */ // each rom contains 8 0x40000 banks, there are 12 roms, so 96 (0x60) banks here!
	ROM_LOAD( "rom6_u65.a1",   0x0000000, 0x0200000, CRC(f33c6c0b) SHA1(06d73cd5b6d27de4d68f2dde1ed4dfa72b9a9178) )
	ROM_LOAD( "rom7_u66.a3",   0x0200000, 0x0200000, CRC(1306f8b3) SHA1(21b0d3180f1f4af77074c35c66844e38a464fea0) )
	ROM_LOAD( "rom8_u61.a4",   0x0400000, 0x0200000, CRC(4f211c00) SHA1(b067de95ad595a4915effefb83789e4e3d9db6f9) )
	ROM_LOAD( "rom9_u62.a6",   0x0600000, 0x0200000, CRC(292d3ef6) SHA1(d027d4c64e57f46e444ee83b62f6c3bdf02e4eed) )
	ROM_LOAD( "rom10_u67.a8",  0x0800000, 0x0200000, CRC(5219bf86) SHA1(946c8fcf3c04a88517d1a66ccd56609d22da945f) )
	ROM_LOAD( "rom11_u68.a10", 0x0a00000, 0x0200000, CRC(56fe4b1d) SHA1(2ea0413b435dd178174eb66d38dc9f7ab3d07ba5) )
	ROM_LOAD( "rom12_u63.a11", 0x0c00000, 0x0200000, CRC(cc48ff18) SHA1(10f6d9f445c9244b797846450f0c94700ccc7367) )
	ROM_LOAD( "rom13_u64.a13", 0x0e00000, 0x0200000, CRC(a3cd181a) SHA1(6a87479c24a61f7ac940e9c9bb62a18f26c9c843) )
	ROM_LOAD( "rom14_u69.a14", 0x1000000, 0x0200000, CRC(5d8cddec) SHA1(1912850d065d4ce1a1cdfd5a704218e660b5345b) )
	ROM_LOAD( "rom15_u70.a16", 0x1200000, 0x0200000, CRC(c75012f5) SHA1(b1ba0abab3eb8e9e3778eecab4951d828c85cecb) )
	ROM_LOAD( "rom16_u71.a18", 0x1400000, 0x0200000, CRC(efd02b0d) SHA1(b23fa3298fc29086f9ab05bc58775ff47b4cb7a9) )
	ROM_LOAD( "rom17_u72.a19", 0x1600000, 0x0200000, CRC(6b8717c3) SHA1(b5b7e35deaa2f34bccd1e83844d4bc0be845d0b8) )
ROM_END

} // anonymous namespace

GAME( 1993, enmadaio,    0,        enmadaio,     enmadaio,   enmadaio_state, init_enmadaio, ROT0,   "Toaplan / Taito",  "Enma Daio (Japan)", 0 ) // TP-031
