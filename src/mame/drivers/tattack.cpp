// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Angelo Salese
/****************************************************************************
    Time Attacker

    driver by Tomasz Slanina
    improvements by Angelo Salese

    Z80A,
    xtal 8MHz,
    dipsw 8-position x2,
    volume pots x6,
    2114 ram x5,
    7910CQ + NE555P sound section,
    SN74198N shifter
    no proms

    TODO:
    - non-tilemap video offsets/sizes are guessworked;
    - random brick flickering effect is guessworked too, leave MACHINE_IMPERFECT_COLORS in until is tested on HW.
    - outputs (coin counter port same as sound writes?);
    - some dipswitches;
    - sound (requires Epson 7910 Multi-Melody emulation?)
    \- victory BGM cuts off too late?

    Connector pinout from manual

          Solder Side      Parts Side
          -----------      ----------
                  GND   1  GND
                  GND   2  GND
                  GND   3  GND
              Speaker   4  Speaker
                 +12V   5  +12V
              Antenna   6  Antenna
                        7
                        8
                  +5V   9  +5V
                 VR a  10  VR b
         2P VR Center  11  1P VR Center
                 Coin  12  2P Serve
           Coin Alarm  13  1P Start
        Counter (Out)  14  2P Start
              TV Sync  15  1P Serve
      Counter (futei)  16  TV G
         Counter (In)  17  TV B
            Medal Out  18  TV R

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "sound/samples.h"
#include "speaker.h"
#include "tilemap.h"


class tattack_state : public driver_device
{
public:
	tattack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_samples(*this,"samples")
	{ }

	void tattack(machine_config &config);

	void init_tattack();

protected:
	virtual void video_start() override;

private:
	void paddle_w(uint8_t data);
	void ball_w(offs_t offset, uint8_t data);
	void brick_dma_w(uint8_t data);
	void sound_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	void tattack_palette(palette_device &palette) const;

	uint32_t screen_update_tattack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tattack_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<samples_device> m_samples;
	tilemap_t *m_tmap;
	uint8_t m_ball_regs[2];
	uint8_t m_paddle_reg;
	int m_paddle_ysize;
	bool m_bottom_edge_enable;
	bool m_bricks_color_bank;

	void draw_gameplay_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_edge_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);

	static constexpr uint8_t white_pen = 0xf;
	static constexpr uint8_t green_pen = 0x5;
	static constexpr uint8_t yellow_pen = 0x7;
	static constexpr uint8_t red_pen = 0x3;
	static constexpr int paddle_xpos = 38;
};



TILE_GET_INFO_MEMBER(tattack_state::get_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index];

	if((color & 1) || (color > 15))
		logerror("COLOR %i\n", color);

	color >>= 1;

	tileinfo.set(0,
			code,
			color,
			0);
}

void tattack_state::draw_edge_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// left column
	bitmap.plot_box(0,16,216,4,white_pen);
	// upper row
	bitmap.plot_box(216,16,6,226,white_pen);
	// right column
	bitmap.plot_box(0,238,216,4,white_pen);
	if(m_bottom_edge_enable == true)
		bitmap.plot_box(paddle_xpos,16,4,226,white_pen);
}

void tattack_state::draw_gameplay_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint16_t ram_base = 0x40+(m_ram[0x33] & 0x10);
	const int x_base = -8;

	// draw brick pattern
	for(uint16_t ram_offs=ram_base;ram_offs<ram_base+0xe;ram_offs++)
	{
		uint8_t cur_column = m_ram[ram_offs];

		for(int bit=7;bit>-1;bit--)
		{
			bool draw_block = ((cur_column >> bit) & 1) == 1;

			// blinking
			// If player hits a blinking brick then a 30 seconds bonus is awarded.
			// Sometimes game forgets to update the location or the blinking itself (both bits 0)
			// can be either intentional or a game bug.
			// TODO: the mask used here is guessed
			if((m_ram[0x33] & 0x3) == 3)
			{
				int blink_row = m_ram[0x2b];
				int blink_col = m_ram[0x2c];

				if(bit == blink_col && (ram_offs & 0xf) == blink_row)
					draw_block = false;
			}

			if(draw_block == true)
			{
				for(int xi=0;xi<3;xi++)
				{
					for(int yi=0;yi<15;yi++)
					{
						int resx = bit*4+xi+160+x_base;
						int resy = (ram_offs & 0xf)*16+yi+16;

						if(cliprect.contains(resx,resy))
							bitmap.pix(resy, resx) = m_bricks_color_bank == true ? red_pen : (bit & 4 ? yellow_pen : green_pen);
					}
				}
			}
		}
	}

	// draw paddle
	if(m_bottom_edge_enable == false)
	{
		for(int xi=0;xi<4;xi++)
			for(int yi=0;yi<m_paddle_ysize;yi++)
			{
				int resx =(paddle_xpos+xi);
				int resy = m_paddle_reg+yi;

				if(cliprect.contains(resx,resy))
					bitmap.pix(resy, resx) = white_pen;
			}
	}
	// draw ball
	for(int xi=0;xi<3;xi++)
		for(int yi=0;yi<3;yi++)
		{
			int resx = m_ball_regs[0]+xi-2+x_base;
			int resy = m_ball_regs[1]+yi;

			if(cliprect.contains(resx,resy))
				bitmap.pix(resy, resx) = (white_pen);
		}
}

uint32_t tattack_state::screen_update_tattack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tmap->mark_all_dirty();
	m_tmap->draw(screen, bitmap, cliprect, 0,0);

	// draw bricks/ball/paddle
	draw_gameplay_bitmap(bitmap, cliprect);
	// draw edges
	// probably enables thru 0xe040?
	draw_edge_bitmap(bitmap,cliprect);

	return 0;
}

void tattack_state::video_start()
{
	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tattack_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
}

void tattack_state::paddle_w(uint8_t data)
{
	m_paddle_reg = data;
}

void tattack_state::ball_w(offs_t offset, uint8_t data)
{
	m_ball_regs[offset] = data;
}

void tattack_state::brick_dma_w(uint8_t data)
{
	// bit 7: 0->1 transfers from RAM to internal video buffer
	// bit 6: bricks color bank
	m_bricks_color_bank = BIT(data,6);
	// bit 5: flip screen
	flip_screen_set(!(data & 0x20));
	// bit 4: x paddle half size
	m_paddle_ysize = data & 0x10 ? 8 : 16;
	// bit 3: enable bottom edge
	m_bottom_edge_enable = BIT(data,3);
//  popmessage("%02x",data&0x7f);
}

void tattack_state::sound_w(uint8_t data)
{
	// bit 4 enabled on coin insertion (coin counter?)
	// bit 3-0 samples enable, @see tattack_sample_names
	for(int i=0;i<4;i++)
	{
		// don't restart playing if it is still enabled (victory BGM relies on this)
		if(data & 1 << i && m_samples->playing(i) == false)
			m_samples->start(i,i);
		//if((data & 1 << i) == 0 && m_samples->playing(i) == true)
		//  m_samples->stop(i);
	}
}

void tattack_state::tattack_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x4000).portr("AN_PADDLE"); // $315, checks again with same memory, loops if different (?)
	map(0x5000, 0x53ff).ram().share("videoram");
	map(0x6000, 0x6000).portr("DSW2");
	map(0x7000, 0x73ff).ram().share("colorram");    // color map ? something else .. only bits 1-3 are used
	map(0xa000, 0xa000).portr("DSW1");       // dsw ? something else ?
	map(0xc000, 0xc000).portr("INPUTS").w(FUNC(tattack_state::sound_w)); // sound
	map(0xc001, 0xc001).w(FUNC(tattack_state::brick_dma_w)); // bit 7 = strobe ($302)
	map(0xc002, 0xc002).nopw(); // same as sound port, outputs?
	map(0xc005, 0xc005).w(FUNC(tattack_state::paddle_w));
	map(0xc006, 0xc007).w(FUNC(tattack_state::ball_w));
	map(0xe000, 0xe3ff).ram().share("ram");
}

static INPUT_PORTS_START( tattack )
	PORT_START("INPUTS")
	PORT_DIPNAME( 0x01, 0x00, "1-01" ) // reset switch?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "1-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "1-03" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "1-04" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Enable green square blocks" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, "Number of bricks to destroy" )
	PORT_DIPSETTING(    0x04, "112" )
	PORT_DIPSETTING(    0x00, "5" ) // testing option?
	PORT_DIPNAME( 0x08, 0x00, "DSW1 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Paddle in middle of screen" )
	PORT_DIPSETTING(    0x00, "Never on screen" )
	PORT_DIPSETTING(    0x10, "Mode 1" ) // - appears after a set number of bricks destroyed, might be same setting
	PORT_DIPSETTING(    0x20, "Mode 2" ) // /
	PORT_DIPSETTING(    0x30, "Always on screen" )
	PORT_DIPNAME( 0x40, 0x00, "DSW1 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2 3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2 5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x80, "7" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Infinite ) )

	PORT_START("AN_PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};



static GFXDECODE_START( gfx_tattack )
	GFXDECODE_ENTRY( "gfx1", 0     , charlayout,  0, 8 )
GFXDECODE_END

void tattack_state::tattack_palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		int r, g, b;
		if (i)
		{
			r = (i & 1) ? 0xff : 0;
			g = (i & 2) ? 0xff : 0;
			b = (i & 4) ? 0xff : 0;
		}
		else
			r = g = b = 128;

		palette.set_pen_color(2 * i, rgb_t(0x00, 0x00, 0xff));
		palette.set_pen_color(2 * i + 1, rgb_t(r, g, b));
	}
}

static const char *const tattack_sample_names[] =
{
	"*tattack",
	"paddle_hit",
	"wall_hit",
	"brick_destroy",
	"win_bgm",
	nullptr
};

void tattack_state::tattack(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8000000 / 2);   /* 4 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &tattack_state::tattack_map);
	m_maincpu->set_vblank_int("screen", FUNC(tattack_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(24, 256-32-1, 13, 256-11-1);
	screen.set_screen_update(FUNC(tattack_state::screen_update_tattack));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_tattack);
	PALETTE(config, "palette", FUNC(tattack_state::tattack_palette), 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(tattack_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.6);

	/* Discrete ???? */
//  DISCRETE(config, m_discrete);
//  m_discrete->set_intf(tattack);
//  m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tattack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom.9a",     0x0000, 0x1000, CRC(47120994) SHA1(b6e90abbc50cba77df4c0aaf50d1f97b99e33b6d) )

	ROM_REGION( 0x800, "melody", 0 ) // Epson 7910CQ Multi-Melody IC
	ROM_LOAD( "7910cq", 0x000, 0x800, NO_DUMP ) // actual size unknown, needs decapping

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rom.6c",     0x0000, 0x1000, CRC(88ce45cf) SHA1(c7a43bfc9e9c2aeb75a98f723558bc88e53401a7) )
ROM_END

void tattack_state::init_tattack()
{
//  uint8_t *rom = memregion("maincpu")->base();

//  rom[0x1b4]=0;
//  rom[0x1b5]=0;

//  rom[0x262]=0;
//  rom[0x263]=0;
//  rom[0x264]=0;

//  rom[0x32a]=0;
//  rom[0x32b]=0;
//  rom[0x32c]=0;

/*
    possible jumps to 0 (protection checks?)

    rom[0x8a]=0;
    rom[0x8b]=0;
    rom[0x8c]=0;

    rom[0x99]=0;
    rom[0x9a]=0;
    rom[0x9b]=0;

    rom[0xd5]=0;
    rom[0xd6]=0;
    rom[0xd7]=0;

    rom[0x65a]=0;
    rom[0x65b]=0;
    rom[0x65c]=0;
*/

}

GAME( 1983?, tattack, 0, tattack, tattack, tattack_state, init_tattack, ROT270, "Shonan", "Time Attacker", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_NO_COCKTAIL )
// there is another undumped version with katakana Shonan logo and black background
