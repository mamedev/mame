// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*******************************************************/
/*                                                     */
/* Yachiyo "Space Stranger/Space Stranger 2"           */
/*                                                     */
/*******************************************************/


/*
********************************************************

Space Stranger hardware info by Guru

This is a black/white Space Invaders clone made by Yachiyo Electric Co. Ltd. using colored cellophane
to make it look colored. This is probably the same game known as Super Space Stranger on the flyer
and also has 'Super Space Stranger' printed on the control panel/marquee.
Super Space Stranger is said to be made by Ban Presto but the hardware is identical to the Yachiyo PCB.
Additionally there is no evidence in the game or on the control panel or marquee or PCB to suggest it
was actually made by them, so this info is probably false or guessed, or perhaps Ban Presto just
distributed it. Either way there is no evidence that Ban Presto manufactured it.


Bottom Board
------------

YD7000-1A
   |------------------------------------------------|
   |    18.432MHz                                   |
   |                  M5L8224              8080     |
   |                                                |
   |          7400     7404                         |
   |                                                |
   |  7474    7474     7408                7400     |
   |                                                |
|--|  7420    7404     7408     74XX       7442     |
|                                                   |
|     7404    74XX     74XX     74XX                |
|5                                         HSS_01   |
|0    7400    74XX     74XX     74XX                |
|P                                                  |
|I    7474    74XX     74XX     74XX       HSS_02   |
|N                                                  |
|     7402    74XX     74XX     74XX                |
|E                                         HSS_03   |
|D    7408    74XX     74XX     74XX                |
|G                                                  |
|E    7404    74XX     74XX     74XX       HSS_04   |
|                                                   |
|C            74XX     74XX     74XX                |
|O                                         HSS_05   |
|N    7408    74XX     74XX     74XX                |
|N                                                  |
|                                          HSS_06   |
|     7432             4116     4116                |
|--|                                                |
   |                   4116     4116       HSS_07   |
   |          74XX                                  |
   |                   4116     4116                |
   |  7400    74XX                         HSS_08   |
   |                   4116     4116                |
   |  10MHz   74XX                                  |
   |                                     DIP24_SKT  |
   |------------------------------------------------|
Notes: All IC's shown.
       Some chips were black but only obfuscated with black paint so easily cleaned off with acetone ^_^
       Some IC's scratched, shown as 74XX
       If you have a Space Stranger with non-scratched chips, please help to complete
       the TTL listing, otherwise when a scratched chip dies it will not be possible
       to repair it even if it is a common chip because it is unknown and dead.

       8080 CPU  - AM8080 or M5L8080 CPU. Clock 2.048MHz (18.432/9 via M5L8224 Clock Generator IC)
       M5L8224   - Mitsubishi M5L8224 Clock Generator/Divider IC. Divider is fixed at divide-by-9
       HSS*      - 2708 EPROMs
       4116      - Mitsubishi M58759S-25 16k x1-bit DRAM compatible with 4116
       DIP24_SKT - Empty DIP24 socket


Top PCB
-------

YD7000-2B
   |----------------------------------------|
   |                                        |
   |                                        |
   |                                        |
   |                4030     4006           |
   |                                        |
   |                                        |
   |  LM380                  LM3900         |
|--|                                        |
|                                           |
|                            LM3900         |
|5    53200   7408                          |
|0                                      VR2 |
|P    74XX    74XX                          |
|I                  SW(5)    LM3900         |
|N    74XX    74XX                          |
|                                           |
|E    74XX    74XX           LM3900         |
|D                                      VR3 |
|G                                          |
|E                           LM3900         |
|                                           |
|C                                      VR7 |
|O                           LM3900         |
|N                                          |
|N                                      VR4 |
|                            LM3900         |
|          74174    7407                    |
|--|                                    VR1 |
   |               53204    SN76477         |
   |                                        |
   |                                    VR5 |
   |       74174   53217  1455  M53210      |
   |                                        |
   |                      1455         `VR6 |
   |                                        |
   |----------------------------------------|
Notes: All IC's shown.
       Some chips were black but only obfuscated with black paint so easily cleaned off with acetone ^_^
       Some IC's scratched, shown as 74XX
       If you have a Space Stranger with non-scratched chips, please help to complete
       the TTL listing, otherwise when a scratched chip dies it will not be possible
       to repair it even if it is a common chip because it is unknown and dead.

       SN76477 - Texas Instruments Complex Sound Generator
       LM3900  - Quad Operational Amplifier
       LM380   - 2.5W Power Amplifier
       1455    - MC1455 equivalent to NE555 Timer
       VR1-7   - Volume Pots
       SW(5)   - 5-position DIP Switch marked 'SW'
       53204   - Mitsubishi branded logic chip equivalent to 7404
       53217   - Mitsubishi branded logic chip equivalent to 7417
       53210   - Mitsubishi branded logic chip equivalent to 7410
       53200   - Mitsubishi branded logic chip equivalent to 7400

********************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "emupal.h"
#include "screen.h"

#include "sstrangr.lh"


class sstrangr_state : public driver_device
{
public:
	sstrangr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_ram(*this, "ram")
	{ }

	void sstrngr2(machine_config &config);
	void sstrangr(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_ram;

	uint8_t m_flip_screen;

	void port_w(uint8_t data);

	uint32_t screen_update_sstrangr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sstrngr2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void sstrangr_io_map(address_map &map);
	void sstrangr_map(address_map &map);
};



/*************************************
 *
 *  Video system
 *
 *************************************/

void sstrangr_state::video_start()
{
	save_item(NAME(m_flip_screen));
}

uint32_t sstrangr_state::screen_update_sstrangr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;

		uint8_t x = offs << 3;
		int y = offs >> 5;
		uint8_t data = m_ram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			if (m_flip_screen)
			{
				pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
				data = data << 1;
			}
			else
			{
				pen = (data & 0x01) ? rgb_t::white() : rgb_t::black();
				data = data >> 1;
			}

			bitmap.pix32(y, x) = pen;

			x = x + 1;
		}
	}

	return 0;
}

uint32_t sstrangr_state::screen_update_sstrngr2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *color_map_base = &memregion("proms")->base()[m_flip_screen ? 0x0000 : 0x0200];

	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;

		offs_t color_address = (offs >> 9 << 5) | (offs & 0x1f);

		uint8_t data = m_ram[offs];
		uint8_t fore_color = color_map_base[color_address] & 0x07;

		for (int i = 0; i < 8; i++)
		{
			uint8_t color;

			if (m_flip_screen)
			{
				color = (data & 0x80) ? fore_color : 0;
				data = data << 1;
			}
			else
			{
				color = (data & 0x01) ? fore_color : 0;
				data = data >> 1;
			}

			bitmap.pix32(y, x) = m_palette->pen_color(color);

			x = x + 1;
		}
	}

	return 0;
}


void sstrangr_state::port_w(uint8_t data)
{
	m_flip_screen = data & 0x20;
}



void sstrangr_state::sstrangr_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram().share("ram");
	map(0x6000, 0x63ff).rom();
}


void sstrangr_state::sstrangr_io_map(address_map &map)
{
	map(0x41, 0x41).portr("DSW");
	map(0x42, 0x42).portr("INPUTS");
	map(0x44, 0x44).portr("EXT").w(FUNC(sstrangr_state::port_w));
}



static INPUT_PORTS_START( sstrangr )
	PORT_START("DSW") // 1 x 5-dip bank
	PORT_DIPNAME( 0x03, 0x01, "Extra Play" )           PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW:5" ) /* Must be ACTIVE_LOW for game to boot */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("EDGE:1")  // This is probably an edge connector pin
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("EXT")      /* External switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


void sstrangr_state::sstrangr(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, XTAL(18'432'000)/9); // M5L8080AP, measured 2047840 Hz
	m_maincpu->set_addrmap(AS_PROGRAM, &sstrangr_state::sstrangr_map);
	m_maincpu->set_addrmap(AS_IO, &sstrangr_state::sstrangr_io_map);
	m_maincpu->set_periodic_int(FUNC(sstrangr_state::irq0_line_hold), attotime::from_hz(2*60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(32*8, 262);     /* vert size is a guess, taken from mw8080bw */
	screen.set_visarea(0*8, 32*8-1, 4*8, 32*8-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(sstrangr_state::screen_update_sstrangr));

	/* sound hardware */
}



/*******************************************************/
/*                                                     */
/* Yachiyo "Space Stranger 2"                          */
/*                                                     */
/*******************************************************/

/* Color version of Space Stranger, board has Stranger 2 written on it */

static INPUT_PORTS_START( sstrngr2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Extra Play" )            PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x01, "3000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("EDGE:1")  // This is probably an edge connector pin
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("EXT")      /* External switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x02, 0x00, "Player's Bullet Speed (Cheat)" )  PORT_DIPLOCATION("EXT:1")  // This is probably an edge connector pin
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


void sstrangr_state::sstrngr2(machine_config &config)
{
	sstrangr(config);

	/* video hardware */
	subdevice<screen_device>("screen")->set_screen_update(FUNC(sstrangr_state::screen_update_sstrngr2));

	PALETTE(config, m_palette, palette_device::RBG_3BIT);
}



ROM_START( sstrangr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hss-01.58",     0x0000, 0x0400, CRC(feec7600) SHA1(787a6be4e24ce931e7678e777699b9f6789bc199) )
	ROM_LOAD( "hss-02.59",     0x0400, 0x0400, CRC(7281ff0b) SHA1(56649d1362be1b9f517cb8616cbf9e4f955e9a2d) )
	ROM_LOAD( "hss-03.60",     0x0800, 0x0400, CRC(a09ec572) SHA1(9c4ad811a6c0460403f9cdc9fe5381c460249ff5) )
	ROM_LOAD( "hss-04.61",     0x0c00, 0x0400, CRC(ec411aca) SHA1(b72eb6f7c3d69e2829280d1ab982099f6eff0bde) )
	ROM_LOAD( "hss-05.62",     0x1000, 0x0400, CRC(7b1b81dd) SHA1(3fa6e244e203fb75f92b19db7b4b18645b3f66a3) )
	ROM_LOAD( "hss-06.63",     0x1400, 0x0400, CRC(de383625) SHA1(7ec0d7171e771c4b43e026f3f50a88d8ab2236bb) )
	ROM_LOAD( "hss-07.64",     0x1800, 0x0400, CRC(2e41d0f0) SHA1(bba720b0c5a7bd47abb8bc8498a989e17dc52428) )
	ROM_LOAD( "hss-08.65",     0x1c00, 0x0400, CRC(bd14d0b0) SHA1(9665f639afef9c1291f2efc054216ff44c595b45) )
ROM_END

ROM_START( sstrangr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4764.09",      0x0000, 0x2000, CRC(d88f86cc) SHA1(9f284ee50caf3c64bd04a79a798de620348881bc) )
	ROM_LOAD( "2708.10",      0x6000, 0x0400, CRC(eba304c1) SHA1(3fa6fbb29fa46c146283f69a712bfc51cbb2a43c) )

	ROM_REGION( 0x0400, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "2708.15",      0x0000, 0x0400, CRC(c176a89d) SHA1(955dd540dc3787091c3f34ae122a13e6b7523414) )
ROM_END


GAMEL( 1978, sstrangr,  0,        sstrangr, sstrangr, sstrangr_state, empty_init, ROT270, "Yachiyo Electronics, Ltd.", "Space Stranger",   MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_sstrangr )
GAME(  1979, sstrangr2, sstrangr, sstrngr2, sstrngr2, sstrangr_state, empty_init, ROT270, "Yachiyo Electronics, Ltd.", "Space Stranger 2", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
