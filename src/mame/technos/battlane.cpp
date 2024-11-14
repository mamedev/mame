// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    Battle Lane Vol. 5
    1986 Taito

    This is a Technos game, though it was distributed by Taito and uses
    some DECO custom ICs.

    Driver provided by Paul Leaman (paul@vortexcomputing.demon.co.uk)

***************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "tilemap.h"


namespace {

class battlane_state : public driver_device
{
public:
	battlane_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void battlane(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_tileram;
	required_shared_ptr<uint8_t> m_spriteram;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	bitmap_ind8 m_screen_bitmap = 0;
	uint8_t m_video_ctrl = 0U;
	uint8_t m_cpu_control = 0U;  // CPU interrupt control register

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void cpu_command_w(uint8_t data);
	void palette_w(offs_t offset, uint8_t data);
	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);
	void bitmap_w(offs_t offset, uint8_t data);
	void video_ctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_2x2);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpu1_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_fg_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};


void battlane_state::palette_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2;

	// red component
	bit0 = (~data >> 0) & 0x01;
	bit1 = (~data >> 1) & 0x01;
	bit2 = (~data >> 2) & 0x01;
	int r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	// green component
	bit0 = (~data >> 3) & 0x01;
	bit1 = (~data >> 4) & 0x01;
	bit2 = (~data >> 5) & 0x01;
	int g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	// blue component
	bit0 = (~m_video_ctrl >> 7) & 0x01;
	bit1 = (~data >> 6) & 0x01;
	bit2 = (~data >> 7) & 0x01;
	int b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

void battlane_state::scrollx_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, ((m_video_ctrl & 0x01) << 8) + data);
}

void battlane_state::scrolly_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, ((m_cpu_control & 0x01) << 8) + data);
}

void battlane_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void battlane_state::bitmap_w(offs_t offset, uint8_t data)
{
	int orval = (~m_video_ctrl >> 1) & 0x07;
	if (!orval)
		orval = 7;

	for (int i = 0; i < 8; i++)
	{
		if (data & 1 << i)
		{
			m_screen_bitmap.pix(offset & 0xff, (offset >> 8) * 8 + i) |= orval;
		}
		else
		{
			m_screen_bitmap.pix(offset & 0xff, (offset >> 8) * 8 + i) &= ~orval;
		}
	}
}

void battlane_state::video_ctrl_w(uint8_t data)
{
	/*
	    Video control register

	        0x80    = low bit of blue component (taken when writing to palette)
	        0x0e    = Bitmap plane (bank?) select  (0-7)
	        0x01    = Scroll MSB
	*/

	m_video_ctrl = data;
}

TILE_GET_INFO_MEMBER(battlane_state::get_tile_info_bg)
{
	int code = m_tileram[tile_index];
	int attr = m_tileram[tile_index + 0x400];
	int gfxn = (attr & 0x01) + 1;
	int color = (attr >> 1) & 0x03;

	tileinfo.set(gfxn, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(battlane_state::tilemap_scan_rows_2x2)
{
	/*
	        Tilemap Memory Organization

	     0              15 16            31
	    +-----------------+----------------+
	    |0              15|256             |0
	    |                 |                |
	    |     screen 0    |    screen 1    |
	    |                 |                |
	    |240           255|             511|15
	    +-----------------+----------------+
	    |512              |768             |16
	    |                 |                |
	    |     screen 2    |    screen 3    |
	    |                 |                |
	    |              767|            1023|31
	    +-----------------+-----------------

	*/

	return (row & 0xf) * 16 + (col & 0xf) + (row & 0x10) * 32 + (col & 0x10) * 16;
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void battlane_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(battlane_state::get_tile_info_bg)), tilemap_mapper_delegate(*this, FUNC(battlane_state::tilemap_scan_rows_2x2)), 16, 16, 32, 32);
	m_screen_bitmap.allocate(32 * 8, 32 * 8);
	save_item(NAME(m_screen_bitmap));
}

void battlane_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x100; offs += 4)
	{
		/*
		    0x80 = Bank 2
		    0x40 =
		    0x20 = Bank 1
		    0x10 = Y Double
		    0x08 = Color
		    0x04 = X Flip
		    0x02 = Y Flip
		    0x01 = Sprite Enable
		*/

		int const attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 3];

		code += 256 * ((attr >> 6) & 0x02);
		code += 256 * ((attr >> 5) & 0x01);

		if (attr & 0x01)
		{
			int const color = BIT(attr, 3);

			int sx = m_spriteram[offs + 2];
			int sy = m_spriteram[offs];

			int flipx = BIT(attr, 2);
			int flipy = BIT(attr, 1);

			if (!flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (attr & 0x10) // Y Double
			{
				if (flip_screen())
					sy += 16;

				for (int i = 0; i < 2; i++)
					m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
						code + i,
						color,
						flipx, flipy,
						sx, sy - (16 * (i ^ flipy)), 0);
			}
			else
			{
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy, 0);
			}
		}
	}
}

void battlane_state::draw_fg_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32 * 8; y++)
	{
		for (int x = 0; x < 32 * 8; x++)
		{
			int const data = m_screen_bitmap.pix(y, x);

			if (data)
			{
				int px = x, py = y;
				if (flip_screen())
				{
					px = 255 - px;
					py = 255 - py;
				}

				if (cliprect.contains(px, py))
					bitmap.pix(py, px) = data;
			}
		}
	}
}

uint32_t battlane_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	draw_fg_bitmap(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void battlane_state::cpu_command_w(uint8_t data)
{
	m_cpu_control = data;

	/*
	  CPU control register

	    0x80    = Video Flip
	    0x08    = NMI
	    0x04    = CPU 0 IRQ   (0 = Activate)
	    0x02    = CPU 1 IRQ   (0 = Activate)
	    0x01    = Y Scroll MSB
	*/

	flip_screen_set(data & 0x80);

	/*
	    I think that the NMI is an inhibitor. It is constantly set
	    to zero whenever NMIs are allowed.

	    However, it could also be that setting to zero could
	    cause the NMI to trigger. I really don't know.
	*/

	/*
	if (~m_cpu_control & 0x08)
	{
	    m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	    m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	*/

	/*
	    CPU2's SWI will trigger an 6809 IRQ on the master by resetting 0x04
	    Master will respond by setting the bit back again
	*/
	m_maincpu->set_input_line(M6809_IRQ_LINE,  data & 0x04 ? CLEAR_LINE : HOLD_LINE);

	/*
	Slave function call (e.g. ROM test):
	FA7F: 86 03       LDA   #$03    ; Function code
	FA81: 97 6B       STA   $6B
	FA83: 86 0E       LDA   #$0E
	FA85: 84 FD       ANDA  #$FD    ; Trigger IRQ
	FA87: 97 66       STA   $66
	FA89: B7 1C 03    STA   $1C03   ; Do Trigger
	FA8C: C6 40       LDB   #$40
	FA8E: D5 68       BITB  $68
	FA90: 27 FA       BEQ   $FA8C   ; Wait for slave IRQ pre-function dispatch
	FA92: 96 68       LDA   $68
	FA94: 84 01       ANDA  #$01
	FA96: 27 FA       BEQ   $FA92   ; Wait for bit to be set
	*/

	m_subcpu->set_input_line(M6809_IRQ_LINE, data & 0x02 ? CLEAR_LINE : HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(battlane_state::cpu1_interrupt)
{
	// See note in cpu_command_w
	if (~m_cpu_control & 0x08)
	{
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void battlane_state::prg_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("work_ram");
	map(0x1000, 0x17ff).ram().w(FUNC(battlane_state::tileram_w)).share(m_tileram);
	map(0x1800, 0x18ff).ram().share(m_spriteram);
	map(0x1c00, 0x1c00).portr("P1").w(FUNC(battlane_state::video_ctrl_w));
	map(0x1c01, 0x1c01).portr("P2").w(FUNC(battlane_state::scrollx_w));
	map(0x1c02, 0x1c02).portr("DSW1").w(FUNC(battlane_state::scrolly_w));
	map(0x1c03, 0x1c03).portr("DSW2").w(FUNC(battlane_state::cpu_command_w));
	map(0x1c04, 0x1c05).rw("ymsnd", FUNC(ym3526_device::read), FUNC(ym3526_device::write));
	map(0x1e00, 0x1e3f).w(FUNC(battlane_state::palette_w));
	map(0x2000, 0x3fff).ram().w(FUNC(battlane_state::bitmap_w)).share("bitmap_ram");
	map(0x4000, 0xffff).rom();
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( battlane )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C )  )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy )  )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard )  )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "20K 50K+" )
	PORT_DIPSETTING(    0x08, "20K 70K+" )
	PORT_DIPSETTING(    0x04, "20K 90K+" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{
		7, 6, 5, 4, 3, 2, 1, 0,
		16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
	},
	{
		15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
		7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8,
	},
	16*8*2
};

static const gfx_layout tilelayout =
{
	16, 16,    // 16*16 tiles
	256,    // 256 tiles
	3,      // 3 bits per pixel
	{ 0x8000*8+4, 4, 0 },    // plane offset
	{
		3, 2, 1, 0,
		8+3, 8+2, 8+1, 8+0,
		16+3, 16+2, 16+1, 16+0,
		16+8+3, 16+8+2, 16+8+1, 16+8+0
	},
	{
		0*8*4, 1*8*4,  2*8*4,  3*8*4,  4*8*4,  5*8*4,  6*8*4,  7*8*4,
		8*8*4, 9*8*4, 10*8*4, 11*8*4, 12*8*4, 13*8*4, 14*8*4, 15*8*4
	},
	8*8*4*2     // every char takes consecutive bytes
};

static const gfx_layout tilelayout2 =
{
	16, 16,    // 16*16 tiles
	256,    // 256 tiles
	3,      // 3 bits per pixel
	{ 0x8000*8, 0x4000*8+4, 0x4000*8+0 },    // plane offset
	{
		3, 2, 1, 0,
		8+3, 8+2, 8+1, 8+0,
		16+3, 16+2, 16+1, 16+0,
		16+8+3, 16+8+2, 16+8+1, 16+8+0
	},
	{
		0*8*4, 1*8*4,  2*8*4,  3*8*4,  4*8*4,  5*8*4,  6*8*4,  7*8*4,
		8*8*4, 9*8*4, 10*8*4, 11*8*4, 12*8*4, 13*8*4, 14*8*4, 15*8*4
	},
	8*8*4*2     // every char takes consecutive bytes
};


static GFXDECODE_START( gfx_battlane )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,  0, 2 )   // colors 0x00-0x0f
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,   32, 4 )   // colors 0x20-0x3f
	GFXDECODE_ENTRY( "tiles",   0, tilelayout2,  32, 4 )   // colors 0x20-0x3f
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void battlane_state::machine_start()
{
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_cpu_control));
}

void battlane_state::machine_reset()
{
	m_video_ctrl = 0;
	m_cpu_control = 0;
}

void battlane_state::battlane(machine_config &config)
{
	// TODO: measure clocks and determine whether CPUs are MC6809 or MC6809E (both are surface-scratched)
	MC6809E(config, m_maincpu, 1'500'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &battlane_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(battlane_state::cpu1_interrupt));

	MC6809E(config, m_subcpu, 1'500'000);
	m_subcpu->set_addrmap(AS_PROGRAM, &battlane_state::prg_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32 * 8, 32 * 8);
	screen.set_visarea(1 * 8, 31 * 8 - 1, 0 * 8, 32 * 8 - 1);
	screen.set_screen_update(FUNC(battlane_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_battlane);
	PALETTE(config, m_palette).set_entries(64);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3526_device &ymsnd(YM3526(config, "ymsnd", 3'000'000));
	ymsnd.irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( battlane )
	ROM_REGION( 0x8000, "user1", 0 )
	ROM_LOAD( "da00-5",     0x0000, 0x8000, CRC(85b4ed73) SHA1(b8e61eedf8fb75bb07f1df91a7465cee2b6ff372) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	// first half of da00-5 will be copied at 0x4000-0x7fff
	ROM_COPY( "user1", 0x0000, 0x4000, 0x4000 )
	ROM_LOAD( "da01-5",        0x8000, 0x8000, CRC(7a6c3f02) SHA1(bee1ee858f81453a53afc2d016f549924801b090) )

	ROM_REGION( 0x10000, "sub", 0 )
	// second half of da00-5 will be copied at 0x4000-0x7fff
	ROM_COPY( "user1", 0x4000, 0x4000, 0x4000 )
	ROM_LOAD( "da02-2",     0x8000, 0x8000, CRC(69d8dafe) SHA1(a7dab13d7f05bf8e3220bb8193066e9b45c86a17) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "da05",      0x00000, 0x8000, CRC(834829d4) SHA1(d56781d2a7ef89b645a637166cd5acde6a65f7f9) ) // Plane 1+2
	ROM_LOAD( "da04",      0x08000, 0x8000, CRC(f083fd4c) SHA1(eb8f079776a0efd898574874d21f865311ecd8ba) ) // Plane 3+4
	ROM_LOAD( "da03",      0x10000, 0x8000, CRC(cf187f25) SHA1(c0d2d85f85340c12c1b61cc062506ffa4841ef78) ) // Plane 5+6

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "da06",      0x00000, 0x8000, CRC(9c6a51b3) SHA1(0d623e8fba9373979a93f97cdfcf311c7e7f561a) )
	ROM_LOAD( "da07",      0x08000, 0x4000, CRC(56df4077) SHA1(f4b8047c3b4d5897ba91489bc76a9504d9941072) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.7h", 0x00000, 0x0020, CRC(b9933663) SHA1(5d5c840caa0b8416ed7dd4890dd5f3e4a9e86511) )
	ROM_LOAD( "82s123.9n", 0x00020, 0x0020, CRC(06491e53) SHA1(d6cf5003798f9a9d555bca97844dcb2966cbac9d) )
ROM_END

ROM_START( battlane2 )
	ROM_REGION( 0x8000, "user1", 0 )
	ROM_LOAD( "da00-3",     0x0000, 0x8000, CRC(7a0a5d58) SHA1(ef97e5a64a668c437c18cda931c52bf39b580b4a) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	// first half of da00-3 will be copied at 0x4000-0x7fff
	ROM_COPY( "user1", 0x0000, 0x4000, 0x4000 )
	ROM_LOAD( "da01-3",     0x8000, 0x8000, CRC(d9e40800) SHA1(dc87ae0d8631c220dbbddbf0e49b6bdaeb635269) )

	ROM_REGION( 0x10000, "sub", 0 )
	// second half of da00-3 will be copied at 0x4000-0x7fff
	ROM_COPY( "user1", 0x4000, 0x4000, 0x4000 )
	ROM_LOAD( "da02-2",     0x8000, 0x8000, CRC(69d8dafe) SHA1(a7dab13d7f05bf8e3220bb8193066e9b45c86a17) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "da05",      0x00000, 0x8000, CRC(834829d4) SHA1(d56781d2a7ef89b645a637166cd5acde6a65f7f9) ) // Plane 1+2
	ROM_LOAD( "da04",      0x08000, 0x8000, CRC(f083fd4c) SHA1(eb8f079776a0efd898574874d21f865311ecd8ba) ) // Plane 3+4
	ROM_LOAD( "da03",      0x10000, 0x8000, CRC(cf187f25) SHA1(c0d2d85f85340c12c1b61cc062506ffa4841ef78) ) // Plane 5+6

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "da06",      0x00000, 0x8000, CRC(9c6a51b3) SHA1(0d623e8fba9373979a93f97cdfcf311c7e7f561a) )
	ROM_LOAD( "da07",      0x08000, 0x4000, CRC(56df4077) SHA1(f4b8047c3b4d5897ba91489bc76a9504d9941072) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.7h", 0x00000, 0x0020, CRC(b9933663) SHA1(5d5c840caa0b8416ed7dd4890dd5f3e4a9e86511) )
	ROM_LOAD( "82s123.9n", 0x00020, 0x0020, CRC(06491e53) SHA1(d6cf5003798f9a9d555bca97844dcb2966cbac9d) )
ROM_END

ROM_START( battlane3 )
	ROM_REGION( 0x8000, "user1", 0 )
	ROM_LOAD( "bl_04.rom",  0x0000, 0x8000, CRC(5681564c) SHA1(25b3a715e91976830d87c7c45b93b473df709241) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	// first half of bl_04.rom will be copied at 0x4000-0x7fff
	ROM_COPY( "user1", 0x0000, 0x4000, 0x4000 )
	ROM_LOAD( "bl_05.rom",  0x8000, 0x8000, CRC(001c4bbe) SHA1(4320c0a85b5b3505ac7292673759e5288cf4187f) )

	ROM_REGION( 0x10000, "sub", 0 )
	// second half of bl_04.rom will be copied at 0x4000-0x7fff
	ROM_COPY( "user1", 0x4000, 0x4000, 0x4000 )
	ROM_LOAD( "da02-2",     0x8000, 0x8000, CRC(69d8dafe) SHA1(a7dab13d7f05bf8e3220bb8193066e9b45c86a17) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "da05",      0x00000, 0x8000, CRC(834829d4) SHA1(d56781d2a7ef89b645a637166cd5acde6a65f7f9) ) // Plane 1+2
	ROM_LOAD( "da04",      0x08000, 0x8000, CRC(f083fd4c) SHA1(eb8f079776a0efd898574874d21f865311ecd8ba) ) // Plane 3+4
	ROM_LOAD( "da03",      0x10000, 0x8000, CRC(cf187f25) SHA1(c0d2d85f85340c12c1b61cc062506ffa4841ef78) ) // Plane 5+6

	ROM_REGION( 0x0c000, "tiles", 0 )
	ROM_LOAD( "da06",      0x00000, 0x8000, CRC(9c6a51b3) SHA1(0d623e8fba9373979a93f97cdfcf311c7e7f561a) )
	ROM_LOAD( "da07",      0x08000, 0x4000, CRC(56df4077) SHA1(f4b8047c3b4d5897ba91489bc76a9504d9941072) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "82s123.7h", 0x00000, 0x0020, CRC(b9933663) SHA1(5d5c840caa0b8416ed7dd4890dd5f3e4a9e86511) )
	ROM_LOAD( "82s123.9n", 0x00020, 0x0020, CRC(06491e53) SHA1(d6cf5003798f9a9d555bca97844dcb2966cbac9d) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, battlane,  0,        battlane, battlane, battlane_state, empty_init, ROT90, "Technos Japan (Taito license)", "Battle Lane! Vol. 5 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, battlane2, battlane, battlane, battlane, battlane_state, empty_init, ROT90, "Technos Japan (Taito license)", "Battle Lane! Vol. 5 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, battlane3, battlane, battlane, battlane, battlane_state, empty_init, ROT90, "Technos Japan (Taito license)", "Battle Lane! Vol. 5 (set 3)", MACHINE_SUPPORTS_SAVE )
