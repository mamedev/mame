// license:BSD-3-Clause
// copyright-holders: Luca Elia

/***************************************************************************

                        -= Sky Fox / Exerizer =-

                driver by   Luca Elia (l.elia@tin.it)


CPU  :  Z80A x 2
Sound:  YM2203C x 2
Other:  2 HM6116LP-3 (one on each board)
        1 KM6264L-15 (on bottom board)

TODO:
- The background rendering is entirely guesswork;
- DMA trigger happens at vpos 24 with legacy video parameters,
  needs H/VSync PCB calculations, also notice that 62.65 Hz comes from the
  bootleg and may be different for original;

2008-07
Verified Dip locations and recommended settings with manual

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class skyfox_state : public driver_device
{
public:
	skyfox_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram"),
		m_bgrom(*this, "tiles")
	{ }

	void skyfox(machine_config &config);

	void init_skyfox();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bgram;
	required_region_ptr<uint8_t> m_bgrom;

	uint8_t m_bg_ctrl = 0;

	void bg_ctrl_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************


                            [ 1 Background ]

    The stars in the background are not tile based (I think!) and
    their rendering is entirely guesswork for now..

    I draw a star for each horizontal line using 2 bytes in the
    background ROM:

    - the first byte seems a color / shape info
    - the second byte seems a position info

    The ROM holds 4 chunks of $2000 bytes. Most of the data does not
    change between chunks, while the remaining part (which is rendered
    to what seems a "milky way") pulsates in color and/or shape
    to simulate the shimmering of stars (?!) if we draw one chunk only
    and cycle through the four. Indeed, there's a register cycling
    through 4 values.

    Since the result kind of matches a screenshot we have, I feel the
    drawn result is not that far from reality. On the other hand we
    have a random arrangement of stars, so it's hard to tell for sure..

                            [ 256 Sprites ]

    Sprites are 8 planes deep and can be 8x8, 16x16 or 32x32 pixels
    in size. They are stored as 32x32x8 tiles in the ROMs.


***************************************************************************/

/***************************************************************************

  Convert the color PROMs into a more useable format.

  There are three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 110 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 680 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

static constexpr res_net_decode_info decode_info =
{
	1,
	0, 255, // start/end
	// R,     G,     B,
	{  0,     0x100, 0x200, }, // offsets
	{  0,     0,     0,     }, // shifts
	{  0xf,   0xf,   0xf,   }  // masks
};

static constexpr res_net_info net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_NONE, 0, 0, 4, { 1200, 680, 220, 110 } },
		{ RES_NET_AMP_NONE, 0, 0, 4, { 1200, 680, 220, 110 } },
		{ RES_NET_AMP_NONE, 0, 0, 4, { 1200, 680, 220, 110 } }
	}
};

void skyfox_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, color_prom, decode_info, net_info);
	palette.set_pen_colors(0, rgb);
}


/***************************************************************************

                                Sprites Drawing

Offset:         Value:

03              Code: selects one of the 32x32 tiles in the ROMs.
                (Tiles $80-ff are bankswitched to cover $180 tiles)

02              Code + Attr

                    7654 ----   Code (low 4 bits)
                                8x8   sprites use bits 7654 (since there are 16 8x8  tiles in the 32x32 one)
                                16x16 sprites use bits --54 (since there are 4 16x16 tiles in the 32x32 one)
                                32x32 sprites use no bits   (since the 32x32 tile is already selected)

                    7--- 3---   Size
                                1--- 1--- : 32x32 sprites
                                0--- 1--- : 16x16 sprites
                                8x8 sprites otherwise

                    ---- -2--   Flip Y
                    ---- --1-   Flip X
                    ---- ---0   X Low Bit

00              Y

01              X (High 8 Bits)

***************************************************************************/

void skyfox_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int const width = m_screen->width();
	int const height = m_screen->height();

	// The 32x32 tiles in the 80-ff range are bankswitched
	int const shift = (m_bg_ctrl & 0x80) ? (4 - 1) : 4;

	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code = m_spriteram[offs + 3] << 8 | m_spriteram[offs + 2];
		int flipx = code & 0x2;
		int flipy = code & 0x4;
		int y = m_spriteram[offs + 0];
		int x = m_spriteram[offs + 1] << 1 | (code & 1);

		int const high_code = ((code >> 4) & 0x7f0) + ((code & 0x8000) >> shift);

		int low_code, n;

		switch (code & 0x88)
		{
			case 0x88:  n = 4; low_code = 0; break;
			case 0x08:  n = 2; low_code = (code & 0x20) >> 2 | (code & 0x10) >> 3; break;
			default:    n = 1; low_code = (code >> 4) & 0xf; break;
		}

		if (m_bg_ctrl & 1) // flipscreen
		{
			x = width  - x - n * 8;
			y = height - y - n * 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		int xstart, ystart, xend, yend, xinc, yinc;

		if (flipx)  { xstart = n - 1;  xend = -1;  xinc = -1; }
		else        { xstart = 0;      xend = n;   xinc = +1; }

		if (flipy)  { ystart = n - 1;  yend = -1;  yinc = -1; }
		else        { ystart = 0;      yend = n;   yinc = +1; }


		code = low_code + high_code;

		for (int dy = ystart; dy != yend; dy += yinc)
		{
			for (int dx = xstart; dx != xend; dx += xinc, code++)
			{
				gfx->transpen(bitmap, cliprect, code, 0, flipx, flipy, dx * 8 + x, dy * 8 + y, 0xff);

				// wraparound y - BTANB: large sprites exiting the screen sometimes reappear on the other edge
				gfx->transpen(bitmap, cliprect, code, 0, flipx, flipy, dx * 8 + x, dy * 8 + y - 256, 0xff);
			}

			if (n == 2)
				code += 2;
		}
	}
}



/***************************************************************************

                            Background Rendering

***************************************************************************/

void skyfox_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Blinking stops until the first star moves after turning on the power
	bool const blinking = (m_bg_ctrl & 0x8);
	/* Star pattern change. This will change when all star clusters go out of screen or when the player restarts.
	   When it changes, it will change color of the star clusters. */
	int const pattern = (m_bg_ctrl & 0x6) >> 1;

	for (int i = 0; i < 0x1000; i++)
	{
		// contains the position of stars from 0xd4e0 in RAM
		int const ramoffset = 0xe0 + (i & 0xf) * 2;
		int const pos = m_bgram[ramoffset + 1] * 2 + ((m_bgram[ramoffset] & 0x80) ? 1 : 0);

		// ROM offset of star pattern
		int const offs = (i * 2) % 0x2000 + pattern * 0x2000;

		/* Adjusted with 1 pixel accuracy compared to PCB.
		   Confirmed that pixel and color pattern match in the "1UP START" screen. */
		int const pen = m_bgrom[offs];
		int const x = m_bgrom[offs + 1] * 2 + pos + 0x5b;
		int const y = (i >> 4) + 1;

		/* When flipscreen is enabled, scroll direction is flipped by only in-game subroutine.
		   This PCB does not seem to support background flip. */

		/* This looks perfect at first glance.
		   but when strict compared on "1UP START" screen,
		   it seems the blinking pattern in each star may be different. */
		if (((m_bg_ctrl >> 4) & 3) != (pen & 3) || !blinking)
			bitmap.pix(y % 256, x % 512) = pen;
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

uint32_t skyfox_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xff, cliprect); // the bg is black
	draw_background(bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/***************************************************************************

                                Main CPU

***************************************************************************/

void skyfox_state::bg_ctrl_w(uint8_t data)
{
	m_bg_ctrl = data;
}

void skyfox_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd3ff).ram().share(m_spriteram);
	map(0xd400, 0xd4ff).ram().share(m_bgram); // For background stars
	// TODO: verify if A11 is unconnected
	map(0xd500, 0xdfff).ram();
	map(0xe000, 0xe000).portr("INPUTS");
	map(0xe001, 0xe001).portr("DSW0");
	map(0xe002, 0xe002).portr("DSW1");
	map(0xe008, 0xe008).w(FUNC(skyfox_state::bg_ctrl_w));
	map(0xe009, 0xe009).w("soundlatch", FUNC(generic_latch_8_device::write));
//  map(0xe00a, 0xe00e) // POST only?
	map(0xe00f, 0xe00f).nopw(); // DMA trigger
	map(0xf001, 0xf001).portr("DSW2");
}


/***************************************************************************

                                Sound CPU

***************************************************************************/

void skyfox_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
//  map(0x9000, 0x9001).nopw(); // ??
	map(0xa000, 0xa001).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
//  map(0xb000, 0xb001).nopw(); // ??
	map(0xc000, 0xc001).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xb000, 0xb000).r("soundlatch", FUNC(generic_latch_8_device::read));
}


/***************************************************************************

                                Input Ports

***************************************************************************/

INPUT_CHANGED_MEMBER(skyfox_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( skyfox )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )       // rest unused?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "20K" )
	PORT_DIPSETTING(    0x08, "30K" )
	PORT_DIPSETTING(    0x10, "40K" )
	PORT_DIPSETTING(    0x18, "50K" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")  // Coins, DSW + Vblank
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	// According to manual, there is also "SW2:4" which has to be always OFF
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")  // DSW
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x05, "5" ) // dupe
	PORT_DIPSETTING(    0x06, "5" ) // dupe
	PORT_DIPSETTING(    0x07, "Infinite (Cheat)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") // Fake input port, coins are directly connected on NMI line
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, skyfox_state, coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, skyfox_state, coin_inserted, 0)
INPUT_PORTS_END




/***************************************************************************

                                Graphics Layouts

***************************************************************************/

/* 8x8x8 tiles (note that the tiles in the ROMs are 32x32x8, but
   we cut them in 8x8x8 ones in the init function, in order to
   support 8x8, 16x16 and 32x32 sprites. */

static GFXDECODE_START( gfx_skyfox )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x8_raw, 0, 1 )
GFXDECODE_END


/***************************************************************************

                                Machine Drivers

***************************************************************************/

void skyfox_state::machine_start()
{
	save_item(NAME(m_bg_ctrl));
}

void skyfox_state::machine_reset()
{
	m_bg_ctrl = 0;
}

void skyfox_state::skyfox(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000) / 2); // Verified at 4MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &skyfox_state::main_map);
	// IM0, never enables ei opcode

	Z80(config, m_audiocpu, XTAL(14'318'181) / 8); // Verified at 1.789772MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &skyfox_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: legacy screen configuration with no vblank irq
	m_screen->set_refresh_hz(62.65);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0+0x60, 320-1+0x60, 0+16, 256-1-16); // from $30*2 to $CC*2+8
	m_screen->set_screen_update(FUNC(skyfox_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skyfox);
	PALETTE(config, m_palette, FUNC(skyfox_state::palette), 256); // 256 static colors

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "mono", 0.80); // Verified at 1.789772MHz

	YM2203(config, "ym2", XTAL(14'318'181) / 8).add_route(ALL_OUTPUTS, "mono", 0.80); // Verified at 1.789772MHz
}



/***************************************************************************

                                ROMs Loading

****************************************************************************

                                    Sky Fox


c042    :   Lives
c044-5  :   Score (BCD)
c048-9  :   Power (BCD)

****************************************************************************

                                Exerizer [Bootleg]

malcor

Location     Type     File ID    Checksum
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TB 5E       27C256      1-J        F302      [  background  ]
TB 5N       27C256      1-I        F5E3      [    sound     ]
LB          27C256      1-A        A53E      [  program 1   ]
LB          27C256      1-B        382C      [  program 2   ]
LB          27C512      1-C        CDAC      [     GFX      ]
LB          27C512      1-D        9C7A      [     GFX      ]
LB          27C512      1-E        D808      [     GFX      ]
LB          27C512      1-F        F87E      [     GFX      ]
LB          27C512      1-G        9709      [     GFX      ]
LB          27C512      1-H        DFDA      [     GFX      ]
TB          82S129      1.BPR      0972      [ video blue   ]
TB          82S129      2.BPR      0972      [ video red    ]
TB          82S129      3.BPR      0972      [ video green  ]

Lower board ROM locations:

---=======------=======----
|    CN2          CN1     |
|                     1-A |
|                         |
|                     1-B |
|                         |
|                         |
|              1 1 1 1 1 1|
|              H G F E D C|
---------------------------

Notes  -  This archive is of a bootleg copy,
       -  Japanese program revision
       -  Although the colour PROMs have the same checksums,
          they are not the same.

Main processor  - Z80 @ 4MHz (8MHz OSC / 2)
Sound processor - Z80 @ 1.789772MHz (14.31818MHz OSC / 8)
                - YM2203C x2 @ 1.789772MHzMHz (14.31818MHz OSC / 8)

Vsync: 62.65hz

  CPU board: Jaleco made in japan ER-8736
Video Board: Jaleco made in japan ER-8737

***************************************************************************/


ROM_START( skyfox )
	ROM_REGION( 0x10000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "skyfox1.bin", 0x00000, 0x8000, CRC(b4d4bb6f) SHA1(ed1cf6d91ca7170cb7d1c80b586c11164430fd49) )
	ROM_LOAD( "skyfox2.bin", 0x08000, 0x8000, CRC(e15e0263) SHA1(005934327834aed46b17161aef82117ee508e9c4) )    // identical halves

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "skyfox9.bin", 0x00000, 0x8000, CRC(0b283bf5) SHA1(5b14d0beea689ee7e9174017e5a127435df4fbe3) )

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "skyfox3.bin", 0x00000, 0x10000, CRC(3a17a929) SHA1(973fb36af416161e04a83d7869819d9b13df18cd) )
	ROM_LOAD( "skyfox4.bin", 0x10000, 0x10000, CRC(358053bb) SHA1(589e3270eda0d44e73fbc7ac06e782f332920b39) )
	ROM_LOAD( "skyfox5.bin", 0x20000, 0x10000, CRC(c1215a6e) SHA1(5ca30be8a68ac6a00907cc9e47ede0acec980f46) )
	ROM_LOAD( "skyfox6.bin", 0x30000, 0x10000, CRC(cc37e15d) SHA1(80806df6185f7b8c2d3ab98420ca514df3e63c8d) )
	ROM_LOAD( "skyfox7.bin", 0x40000, 0x10000, CRC(fa2ab5b4) SHA1(c0878b25dae28f7d49e14376ff885d1d4e3d5dfe) )
	ROM_LOAD( "skyfox8.bin", 0x50000, 0x10000, CRC(0e3edc49) SHA1(3d1c59ecaabe1c9517203b7e814db41d5cff0cd4) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "skyfox10.bin", 0x0000, 0x8000, CRC(19f58f9c) SHA1(6887216243b47152129448cbb4c7d52309feed03) )

	ROM_REGION( 0x300, "proms", 0 ) // colors
	ROM_LOAD( "sfoxrprm.bin", 0x000, 0x100, CRC(79913c7f) SHA1(e64e6a3eb55f37984cb2597c8ffba6bc3bad49c7) )  // R
	ROM_LOAD( "sfoxgprm.bin", 0x100, 0x100, CRC(fb73d434) SHA1(4a9bd61fbdce9441753c5921f95ead5c4655957e) )  // G
	ROM_LOAD( "sfoxbprm.bin", 0x200, 0x100, CRC(60d2ab41) SHA1(e58a54f2aaee5c07136d5437e513d61fb18fbd9f) )  // B
ROM_END

ROM_START( exerizer )
	ROM_REGION( 0x10000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "1.2v",        0x00000, 0x8000, CRC(5df72a5d) SHA1(ca35ac06f3702fd650a584da2f442fbc61c00fce) )
	ROM_LOAD( "2.3v",        0x08000, 0x8000, CRC(e15e0263) SHA1(005934327834aed46b17161aef82117ee508e9c4) )    // 1-b

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "9.5n",        0x00000, 0x8000, CRC(0b283bf5) SHA1(5b14d0beea689ee7e9174017e5a127435df4fbe3) )    // 1-i

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "3-1.7w",      0x00000, 0x10000, CRC(3a17a929) SHA1(973fb36af416161e04a83d7869819d9b13df18cd) )
	ROM_LOAD( "4.7u",        0x10000, 0x10000, CRC(358053bb) SHA1(589e3270eda0d44e73fbc7ac06e782f332920b39) )   // 1-d
	ROM_LOAD( "5-1.7t",      0x20000, 0x10000, CRC(c1215a6e) SHA1(5ca30be8a68ac6a00907cc9e47ede0acec980f46) )
	ROM_LOAD( "6.7s",        0x30000, 0x10000, CRC(cc37e15d) SHA1(80806df6185f7b8c2d3ab98420ca514df3e63c8d) )   // 1-f
	ROM_LOAD( "7.7p",        0x40000, 0x10000, CRC(c9bbfe5c) SHA1(ce3f7d32baa8bb0bfc110877b5b5f4648ee959ac) )
	ROM_LOAD( "8.7n",        0x50000, 0x10000, CRC(0e3edc49) SHA1(3d1c59ecaabe1c9517203b7e814db41d5cff0cd4) )   // 1-h

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "10.5e",       0x0000, 0x8000, CRC(19f58f9c) SHA1(6887216243b47152129448cbb4c7d52309feed03) ) // 1-j

	ROM_REGION( 0x300, "proms", 0 ) // colors
	ROM_LOAD( "r.1c",        0x000, 0x100, CRC(79913c7f) SHA1(e64e6a3eb55f37984cb2597c8ffba6bc3bad49c7) )   // 2-bpr
	ROM_LOAD( "g.1b",        0x100, 0x100, CRC(fb73d434) SHA1(4a9bd61fbdce9441753c5921f95ead5c4655957e) )   // 3-bpr
	ROM_LOAD( "b.1d",        0x200, 0x100, CRC(60d2ab41) SHA1(e58a54f2aaee5c07136d5437e513d61fb18fbd9f) )   // 1-bpr
ROM_END

ROM_START( exerizerb )
	ROM_REGION( 0x10000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "1-a",         0x00000, 0x8000, CRC(5df72a5d) SHA1(ca35ac06f3702fd650a584da2f442fbc61c00fce) )
	ROM_LOAD( "skyfox2.bin", 0x08000, 0x8000, CRC(e15e0263) SHA1(005934327834aed46b17161aef82117ee508e9c4) )    // 1-b

	ROM_REGION( 0x10000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "skyfox9.bin", 0x00000, 0x8000, CRC(0b283bf5) SHA1(5b14d0beea689ee7e9174017e5a127435df4fbe3) )    // 1-i

	ROM_REGION( 0x60000, "sprites", 0 )
	ROM_LOAD( "1-c",         0x00000, 0x10000, CRC(450e9381) SHA1(f99b2ca73f1e4ba91b8066bb6d28d33b66a3ee81) )
	ROM_LOAD( "skyfox4.bin", 0x10000, 0x10000, CRC(358053bb) SHA1(589e3270eda0d44e73fbc7ac06e782f332920b39) )   // 1-d
	ROM_LOAD( "1-e",         0x20000, 0x10000, CRC(50a38c60) SHA1(a4b8d530914d6c85b15940a7821b4365068de668) )
	ROM_LOAD( "skyfox6.bin", 0x30000, 0x10000, CRC(cc37e15d) SHA1(80806df6185f7b8c2d3ab98420ca514df3e63c8d) )   // 1-f
	ROM_LOAD( "1-g",         0x40000, 0x10000, CRC(c9bbfe5c) SHA1(ce3f7d32baa8bb0bfc110877b5b5f4648ee959ac) )
	ROM_LOAD( "skyfox8.bin", 0x50000, 0x10000, CRC(0e3edc49) SHA1(3d1c59ecaabe1c9517203b7e814db41d5cff0cd4) )   // 1-h

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "skyfox10.bin", 0x0000, 0x8000, CRC(19f58f9c) SHA1(6887216243b47152129448cbb4c7d52309feed03) )    // 1-j

	ROM_REGION( 0x300, "proms", 0 ) // colors
	ROM_LOAD( "sfoxrprm.bin", 0x000, 0x100, CRC(79913c7f) SHA1(e64e6a3eb55f37984cb2597c8ffba6bc3bad49c7) )  // 2-bpr
	ROM_LOAD( "sfoxgprm.bin", 0x100, 0x100, CRC(fb73d434) SHA1(4a9bd61fbdce9441753c5921f95ead5c4655957e) )  // 3-bpr
	ROM_LOAD( "sfoxbprm.bin", 0x200, 0x100, CRC(60d2ab41) SHA1(e58a54f2aaee5c07136d5437e513d61fb18fbd9f) )  // 1-bpr
ROM_END


// Untangle the graphics: cut each 32x32x8 tile in 16 8x8x8 tiles
void skyfox_state::init_skyfox()
{
	uint8_t *rom = memregion("sprites")->base();
	uint8_t *end = rom + memregion("sprites")->bytes();
	uint8_t buf[32 * 32];

	while (rom < end)
	{
		for (int i = 0; i < (32 * 32); i++)
			buf[i] = rom[(i % 8) + ((i / 8) % 8) * 32 + ((i / 64) % 4) * 8 + (i / 256) * 256];

		memcpy(rom, buf, 32 * 32);
		rom += 32 * 32;
	}
}

} // anonymous namespace


GAME( 1987, skyfox,    0,      skyfox, skyfox, skyfox_state, init_skyfox, ROT90, "Jaleco (Nichibutsu USA license)", "Sky Fox",            MACHINE_SUPPORTS_SAVE )
GAME( 1987, exerizer,  skyfox, skyfox, skyfox, skyfox_state, init_skyfox, ROT90, "Jaleco",                          "Exerizer (Japan)",   MACHINE_SUPPORTS_SAVE )
GAME( 1987, exerizerb, skyfox, skyfox, skyfox, skyfox_state, init_skyfox, ROT90, "bootleg",                         "Exerizer (bootleg)", MACHINE_SUPPORTS_SAVE )
