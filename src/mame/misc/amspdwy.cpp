// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= American Speedway =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU  :  Z80A x 2
Sound:  YM2151


(c)1987 Enerdyne Technologies, Inc. / PGD

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class amspdwy_state : public driver_device
{
public:
	amspdwy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ym2151(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_io_analog(*this, "AN%u", 1U),
		m_io_wheel(*this, "WHEEL%u", 1U),
		m_io_in0(*this, "IN0")
	{ }

	void amspdwy(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym2151_device> m_ym2151;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* I/O ports */
	required_ioport_array<2> m_io_analog;
	required_ioport_array<2> m_io_wheel;
	required_ioport m_io_in0;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int m_flipscreen;

	/* misc */
	uint16_t m_wheel_old[2];
	uint8_t m_wheel_return[2];

	void amspdwy_flipscreen_w(uint8_t data);
	void amspdwy_videoram_w(offs_t offset, uint8_t data);
	void amspdwy_colorram_w(offs_t offset, uint8_t data);
	uint8_t amspdwy_sound_r();
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_cols_back);

	uint32_t screen_update_amspdwy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <unsigned Index> uint8_t amspdwy_wheel_r();

	void amspdwy_map(address_map &map) ATTR_COLD;
	void amspdwy_portmap(address_map &map) ATTR_COLD;
	void amspdwy_sound_map(address_map &map) ATTR_COLD;
};



void amspdwy_state::amspdwy_flipscreen_w(uint8_t data)
{
	m_flipscreen ^= 1;
	flip_screen_set(m_flipscreen);
}

/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

    Videoram:   76543210    Code Low Bits
    Colorram:   765-----
                ---43---    Code High Bits
                -----210    Color

***************************************************************************/

TILE_GET_INFO_MEMBER(amspdwy_state::get_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	uint8_t color = m_colorram[tile_index];
	tileinfo.set(0,
			code + ((color & 0x18)<<5),
			color & 0x07,
			0);
}

void amspdwy_state::amspdwy_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void amspdwy_state::amspdwy_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


/* logical (col,row) -> memory offset */
TILEMAP_MAPPER_MEMBER(amspdwy_state::tilemap_scan_cols_back)
{
	return col * num_rows + (num_rows - row - 1);
}


void amspdwy_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(amspdwy_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(amspdwy_state::tilemap_scan_cols_back)), 8, 8, 0x20, 0x20);
}



/***************************************************************************

                                Sprites Drawing

Offset:     Format:     Value:

0                       Y
1                       X
2                       Code Low Bits
3           7-------    Flip X
            -6------    Flip Y
            --5-----
            ---4----    ?
            ----3---    Code High Bit?
            -----210    Color

***************************************************************************/

void amspdwy_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int const max_x = m_screen->width()  - 1;
	int const max_y = m_screen->height() - 1;

	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int y = m_spriteram[i + 0];
		int x = m_spriteram[i + 1];
		int const code = m_spriteram[i + 2];
		int const attr = m_spriteram[i + 3];
		int flipx = attr & 0x80;
		int flipy = attr & 0x40;

		if (flip_screen())
		{
			x = max_x - x - 8;
			y = max_y - y - 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
//              code + ((attr & 0x18)<<5),
				code + ((attr & 0x08)<<5),
				attr,
				flipx, flipy,
				x,y,0 );
	}
}




/***************************************************************************

                                Screen Drawing

***************************************************************************/

uint32_t amspdwy_state::screen_update_amspdwy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}



/***************************************************************************


                                    Main CPU


***************************************************************************/

/*
    765-----    Buttons
    ---4----    Sgn(Wheel Delta)
    ----3210    Abs(Wheel Delta)

    Or last value when wheel delta = 0
*/

template <unsigned Index>
uint8_t amspdwy_state::amspdwy_wheel_r()
{
	uint16_t wheel = m_io_analog[Index]->read();
	if (wheel != m_wheel_old[Index])
	{
		wheel = (wheel & 0x7fff) - (wheel & 0x8000);
		if (wheel > m_wheel_old[Index])
			m_wheel_return[Index] = ((+wheel) & 0xf) | 0x00;
		else
			m_wheel_return[Index] = ((-wheel) & 0xf) | 0x10;

		m_wheel_old[Index] = wheel;
	}
	return m_wheel_return[Index] | m_io_wheel[Index]->read();
}

uint8_t amspdwy_state::amspdwy_sound_r()
{
	return (m_ym2151->status_r() & ~0x30) | m_io_in0->read();
}

void amspdwy_state::amspdwy_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x801f).w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x9000, 0x93ff).mirror(0x0400).ram().w(FUNC(amspdwy_state::amspdwy_videoram_w)).share("videoram");
	map(0x9800, 0x9bff).ram().w(FUNC(amspdwy_state::amspdwy_colorram_w)).share("colorram");
	map(0x9c00, 0x9fff).ram(); // unused?
//  map(0xa000, 0xa000).nopw(); // ?
	map(0xa000, 0xa000).portr("DSW1");
	map(0xa400, 0xa400).portr("DSW2").w(FUNC(amspdwy_state::amspdwy_flipscreen_w));
	map(0xa800, 0xa800).r(FUNC(amspdwy_state::amspdwy_wheel_r<0>)); // player 1
	map(0xac00, 0xac00).r(FUNC(amspdwy_state::amspdwy_wheel_r<1>)); // player 2
	map(0xb000, 0xb000).nopw(); // irq ack?
	map(0xb400, 0xb400).r(FUNC(amspdwy_state::amspdwy_sound_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xc000, 0xc0ff).ram().share("spriteram");
	map(0xe000, 0xe7ff).ram();
}

void amspdwy_state::amspdwy_portmap(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("tracks", 0);
}



/***************************************************************************


                                Sound CPU


***************************************************************************/

void amspdwy_state::amspdwy_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
//  map(0x8000, 0x8000).nopw(); // ? writes 0 at start
	map(0x9000, 0x9000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xa000, 0xa001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc000, 0xdfff).ram();
	map(0xffff, 0xffff).nopr(); // ??? IY = FFFF at the start ?
}




/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( amspdwy )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Character Test" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Show Arrows" )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_HIGH, "SW1:5" )
	PORT_DIPNAME( 0x10, 0x00, "Steering Test" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:3" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:1" )        /* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x00, "Time To Qualify" )       PORT_DIPLOCATION("SW2:3,4") /* code at 0x1770 */
	PORT_DIPSETTING(    0x30, "20 sec" )
	PORT_DIPSETTING(    0x20, "30 sec" )
	PORT_DIPSETTING(    0x10, "45 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:2" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:1" )        /* Listed as "Unused" */

	PORT_START("WHEEL1")    // Player 1 Wheel + Coins
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )   // wheel
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2) // 2-3f

	PORT_START("WHEEL2")    // Player 2 Wheel + Coins
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)

	PORT_START("IN0")   // Player 1&2 Pedals + YM2151 Sound Status
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM )

	PORT_START("AN1")   // Player 1 Analog Fake Port
	PORT_BIT( 0xffff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_PLAYER(1)

	PORT_START("AN2")   // Player 2 Analog Fake Port
	PORT_BIT( 0xffff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(20) PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_G) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( amspdwya )
	PORT_INCLUDE(amspdwy)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x00, "Time To Qualify" )       PORT_DIPLOCATION("SW2:4") /* code at 0x2696 */
	PORT_DIPSETTING(    0x10, "45 sec" )
	PORT_DIPSETTING(    0x00, "60 sec" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:3" )        /* Listed as "Unused" */
INPUT_PORTS_END



/***************************************************************************


                                Graphics Layouts


***************************************************************************/

static const gfx_layout layout_8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_amspdwy )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x2, 0, 8 )
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/


void amspdwy_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_wheel_old));
	save_item(NAME(m_wheel_return));
}

void amspdwy_state::machine_reset()
{
	m_flipscreen = 0;
	m_wheel_old[0] = 0;
	m_wheel_old[1] = 0;
	m_wheel_return[0] = 0;
	m_wheel_return[1] = 0;
}

void amspdwy_state::amspdwy(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &amspdwy_state::amspdwy_map);
	m_maincpu->set_addrmap(AS_IO, &amspdwy_state::amspdwy_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(amspdwy_state::irq0_line_hold)); /* IRQ: 60Hz, NMI: retn */

	Z80(config, m_audiocpu, 3000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &amspdwy_state::amspdwy_sound_map);

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0+16, 256-16-1);
	m_screen->set_screen_update(FUNC(amspdwy_state::screen_update_amspdwy));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_amspdwy);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233_inverted, 32);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2151(config, m_ym2151, 3000000);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, 0);
	m_ym2151->add_route(0, "speaker", 1.0, 0);
	m_ym2151->add_route(1, "speaker", 1.0, 1);
}




/***************************************************************************


                                ROMs Loading


***************************************************************************/



/***************************************************************************

                            American Speedway

USES TWO Z80 CPU'S W/YM2151 SOUND
THE NUMBERS WITH THE NAMES ARE PROBABLY CHECKSUMS

NAME    LOCATION    TYPE
------------------------
AUDI9363 U2         27256   CONN BD
GAME5807 U33         "       "
TRKS6092 U34         "       "
HIHIE12A 4A         2732    REAR BD
HILO9B3C 5A          "       "
LOHI4644 2A          "       "
LOLO1D51 1A          "       "

                        American Speedway (Set 2)

1987 Enerdyne Technologies, Inc. Has Rev 4 PGD written on the top board.

Processors
------------------
Dual Z80As
YM2151     (sound)

RAM
------------------
12 2114
5  82S16N

Eproms
==================

Name        Loc   TYpe   Checksum
----------  ----  -----  --------
Game.u22    U33   27256  A222
Tracks.u34  U34   27256  6092
Audio.U02   U2    27256  9363
LOLO1.1A    1A    2732   1D51
LOHI.2A     2A    2732   4644
HIHI.4A     3/4A  2732   E12A
HILO.5A     5A    2732   9B3C

***************************************************************************/

ROM_START( amspdwy )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "game5807.u33", 0x0000, 0x8000, CRC(88233b59) SHA1(bfdf10dde1731cde5c579a9a5173cafe9295a80c) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound Z80 Code */
	ROM_LOAD( "audi9463.u2",  0x0000, 0x8000, CRC(61b0467e) SHA1(74509e7712838dd760919893aeda9241d308d0c3) )

	ROM_REGION( 0x8000, "tracks", 0 )
	ROM_LOAD( "trks6092.u34", 0x0000, 0x8000, CRC(74a4e7b7) SHA1(b4f6e3faaf048351c6671205f52378a64b81bcb1) )

	ROM_REGION( 0x4000, "gfx1", 0 ) /* Layer + Sprites */
	ROM_LOAD( "hilo9b3c.5a",  0x0000, 0x1000, CRC(f50f864c) SHA1(5b2412c1558b30a04523fcdf1d5cf6fdae1ba88d) )
	ROM_LOAD( "hihie12a.4a",  0x1000, 0x1000, CRC(3d7497f3) SHA1(34820ba42d9c9dab1d6fdda15795450ce08392c1) )
	ROM_LOAD( "lolo1d51.1a",  0x2000, 0x1000, CRC(58701c1c) SHA1(67b476e697652a6b684bd76ae6c0078ed4b3e3a2) )
	ROM_LOAD( "lohi4644.2a",  0x3000, 0x1000, CRC(a1d802b1) SHA1(1249ce406b1aa518885a02ab063fa14906ccec2e) )
ROM_END

ROM_START( amspdwya )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "game.u33",     0x0000, 0x8000, CRC(facab102) SHA1(e232969eaaad8b89ac8e28ee0a7996107a7de9a2) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound Z80 Code */
	ROM_LOAD( "audi9463.u2",  0x0000, 0x8000, CRC(61b0467e) SHA1(74509e7712838dd760919893aeda9241d308d0c3) )

	ROM_REGION( 0x8000, "tracks", 0 )
	ROM_LOAD( "trks6092.u34", 0x0000, 0x8000, CRC(74a4e7b7) SHA1(b4f6e3faaf048351c6671205f52378a64b81bcb1) )

	ROM_REGION( 0x4000, "gfx1", 0 ) /* Layer + Sprites */
	ROM_LOAD( "hilo9b3c.5a",  0x0000, 0x1000, CRC(f50f864c) SHA1(5b2412c1558b30a04523fcdf1d5cf6fdae1ba88d) )
	ROM_LOAD( "hihie12a.4a",  0x1000, 0x1000, CRC(3d7497f3) SHA1(34820ba42d9c9dab1d6fdda15795450ce08392c1) )
	ROM_LOAD( "lolo1d51.1a",  0x2000, 0x1000, CRC(58701c1c) SHA1(67b476e697652a6b684bd76ae6c0078ed4b3e3a2) )
	ROM_LOAD( "lohi4644.2a",  0x3000, 0x1000, CRC(a1d802b1) SHA1(1249ce406b1aa518885a02ab063fa14906ccec2e) )
ROM_END

} // anonymous namespace


/* (C) 1987 ETI 8402 MAGNOLIA ST. #C SANTEE, CA 92071 */

GAME( 1987, amspdwy,  0,       amspdwy, amspdwy,  amspdwy_state, empty_init, ROT0, "Enerdyne Technologies Inc.", "American Speedway (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, amspdwya, amspdwy, amspdwy, amspdwya, amspdwy_state, empty_init, ROT0, "Enerdyne Technologies Inc.", "American Speedway (set 2)", MACHINE_SUPPORTS_SAVE )
