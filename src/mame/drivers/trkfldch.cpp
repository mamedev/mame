// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Track & Field Challenge TV Game
https://www.youtube.com/watch?v=wjn1lLylqog

Uses epoxy blobs for CPU etc.
These have been identified as Winbond 2005 BA5962 (large glob) + Winbond 200506 BA5934 (smaller glob)
seems to be G65816 derived with custom vectors?

PCB               Game
TV0001 R1.1       My First DDR
TV0002 R1.0       Track & Field

DDR & TF PCBs look identical, all the parts are in the same place, the traces are the same, and the silkscreened part # for resistors and caps are the same.

Some of m_unkregs must retain value (or return certain things) or RAM containing vectors gets blanked and game crashes.
The G65816 code on these is VERY ugly and difficult to follow, many redundant statements, excessive mode switching, accessing things via pointers to pointers etc.

One of the vectors points to 0x6000, there is nothing mapped there, could it be a small internal ROM or some debug trap for development?

*/

#include "emu.h"

#include "cpu/g65816/g65816.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"


class trkfldch_state : public driver_device
{
public:
	trkfldch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_palram(*this, "palram"),
		m_palette(*this, "palette")
	{ }

	void trkfldch(machine_config &config);
	void vectors_map(address_map &map);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_palram;
	required_device<palette_device> m_palette;

	void draw_sprites(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int pri);
	void render_text_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, uint16_t base);
	void render_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int which);
	uint32_t screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trkfldch_map(address_map &map);

	DECLARE_READ8_MEMBER(read_vector);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint8_t m_which_vector;

	DECLARE_READ8_MEMBER(unkregs_r);
	DECLARE_WRITE8_MEMBER(unkregs_w);

	uint8_t m_unkregs[0x100];

	uint8_t m_unkdata[0x100000];
	int m_unkdata_addr;

};

void trkfldch_state::video_start()
{
}

void trkfldch_state::render_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int which)
{
//	tilemap 0 = trkfld events, my1stddr background 

//	tilemap 1=  my1stddr HUD layer

//	why does this use a different set of scroll registers? maybe global (applies to both layers?) as only one is enabled at this point.
//	uint16_t xscroll = (m_unkregs[0x26] << 0) | (m_unkregs[0x27] << 8); // trkfld tilemap 0, race top
//	uint16_t xscroll = (m_unkregs[0x28] << 0) | (m_unkregs[0x29] << 8); // trkfld tilemap 0, race bot (window?)

//	uint16_t xscroll = (m_unkregs[0x2a] << 0) | (m_unkregs[0x2b] << 8); // trkfld tilemap 0?, javelin
//	uint16_t yscroll = (m_unkregs[0x30] << 0) | (m_unkregs[0x31] << 8); // trkfld tilemap 0, javelin, holes, hammer throw
//	uint16_t xscroll = (m_unkregs[0x3a] << 0) | (m_unkregs[0x3b] << 8); // trkfld tilemap 1?, javelin
//	uint16_t yscroll = (m_unkregs[0x42] << 0) | (m_unkregs[0x43] << 8); // my1stddr tilemap 1 scroller, trkfld tilemap 1 holes (both window)

// for left / right on tilemap 1
//	0x14 & 0x28  (20 and 40) on trkfld hurdle the holes (resets to 0x00 & 0x28 after event - possible default values)
//	0x29 & 0x29 when unused on my1stddr, 0x25 & 0x29 when used
//	uint8_t windowleft = m_unkregs[0x36];
//	uint8_t windowright = m_unkregs[0x37];

//  for top/bottom on tilemap 1
//	uint8_t windowtop = m_unkregs[0x33];
//	uint8_t windowbottom = m_unkregs[0x34];


//	printf("xscroll %04x\n", xscroll);
//	printf("yscroll %04x\n", yscroll);
//	printf("window left/right %02x %02x\n", windowleft, windowright);

//	printf("window top/bottom %02x %02x\n", windowtop, windowbottom);


	int base, gfxbase, gfxregion;
	int tilexsize = 8;

	if (which == 0)
	{
		base = (m_unkregs[0x54] << 8);
		gfxbase = (m_unkregs[0x11] * 0x2000);
	}
	else //if (which == 1)
	{
		base = (m_unkregs[0x55] << 8);
		gfxbase = (m_unkregs[0x13] * 0x2000);
	}

	if (m_unkregs[0x10] & 1) // seems like it might be a global control for bpp?
	{
		gfxbase -= 0x200;
		gfxregion = 0;
	}
	else
	{
		gfxbase -= 0x2ac;
		gfxregion = 2;
	}



	for (int y = 0; y < 30; y++)
	{
		for (int x = 0; x < 41; x++)
		{
			// fppt tttt   tttt tttt

			rectangle clip;
			clip.set(x*8, (x*8)+7, y*8, (y*8)+7);

			clip &= cliprect;


			address_space &mem = m_maincpu->space(AS_PROGRAM);

			int tile_address = (y * 41) + x;

			uint8_t byte = mem.read_byte(base+((tile_address * 2)));
			uint8_t attr = mem.read_byte(base+((tile_address * 2)+1));

			int tile = (attr << 8) | byte;

			int flipx = (tile & 0x8000) >> 15;
			int pal =   (tile & 0x6000) >> 13;

			tile &= 0x1fff;

			tile += gfxbase;

			gfx_element* gfx = m_gfxdecode->gfx(gfxregion);

			gfx->transpen(bitmap, clip,tile, pal, flipx, 0, x*tilexsize, y*8, 0);

		}
	}
}

void trkfldch_state::render_text_tile_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, uint16_t base)
{
	// this isn't correct, it doesn't seem like a 'real' tilemap, maybe some kind of sprite / tile hybrid, or something with end of line markers?
	// it is needed for the 'good' 'perfect' 'miss' text on DDR ingame
	if (0)
	{
		// guess, but it fits with where the other tilegfxbase registers are, and is only written on my1stddr when this layer is enabled
		int tilegfxbase = (m_unkregs[0x17] * 0x80) - 0x100;

		int offs = 0;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				address_space& mem = m_maincpu->space(AS_PROGRAM);

				uint8_t byte = mem.read_byte(base + offs);
				offs++;

				int tile = tilegfxbase | byte;

				gfx_element* gfx = m_gfxdecode->gfx(4);

				gfx->transpen(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 16, 0);

			}
		}
	}
}



// regs                        11 13 15 17
// 
// DDR Title = 5000            03 03 05 00
// DDR Ingame Girl 1 = 7000    04 05 06 80
// DDR Music Select  = 9000    05 03 04 00
// trkfldch logos    = b000    06 07 06 00
// trkfldch ingame uses different bpp so different addressing here too?


void trkfldch_state::draw_sprites(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int pri)
{
	for (int i = 0x500 - 5; i >= 0; i -= 5)
	{
		int priority = (m_spriteram[i + 4] & 0x20)>>5;
		
		// list is NOT drawn but instead z sorted, see shadows in trkfldch
		if (priority != pri)
			continue;

		// logerror("entry %02x %02x %02x %02x %02x\n", m_spriteram[i + 0], m_spriteram[i + 1], m_spriteram[i + 2], m_spriteram[i + 3], m_spriteram[i + 4]);
		int tilegfxbase = (m_unkregs[0x15] * 0x800);

		// --pp tt-y    yyyy yyyy    tttt tttt    yyyy yyyy    --zf -t-x

		int y = m_spriteram[i + 1];
		int x = m_spriteram[i + 3];
		int tile = m_spriteram[i + 2];

		int tilehigh = m_spriteram[i + 4] & 0x04;
		int tilehigh2 = m_spriteram[i + 0] & 0x04;
		int tilehigh3 = m_spriteram[i + 0] & 0x08;

		int pal = 0;

		int flipx = m_spriteram[i + 4] & 0x10;

		if (tilehigh)
			tile += 0x100;

		if (tilehigh2)
			tile += 0x200;

		if (tilehigh3)
			tile += 0x400;

		int xhigh = m_spriteram[i + 4] & 0x01;
		int yhigh = m_spriteram[i + 0] & 0x01; // or enable bit?

		x = x | (xhigh << 8);
		y = y | (yhigh << 8);

		y -= 0x100;
		y -= 16;
		x -= 16;

		gfx_element* gfx;

		if (m_unkregs[0x10] & 1) // seems like it might be a global control for bpp?
		{
			gfx = m_gfxdecode->gfx(1);
			tilegfxbase -= 0x80;
		}
		else
		{
			pal = (m_spriteram[i + 0] & 0x30)>>4;
			gfx = m_gfxdecode->gfx(3);
			tilegfxbase -= 0x40;
			tilegfxbase -= 0x6b;

		}


		gfx->transpen(bitmap, cliprect, tile + tilegfxbase, pal, flipx, 0, x, y, 0);
	}
}

uint32_t trkfldch_state::screen_update_trkfldch(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	bitmap.fill(0, cliprect);

	// 3 lots of 0x100 values, but maybe not rgb, seems to be yuv (borrowed from tetrisp2.cpp)
	for (int i = 0; i < 256; i++)
	{
		uint8_t u =  m_palram[0x000 + i] & 0xff;
		uint8_t y1 = m_palram[0x100 + i] & 0xff;
		uint8_t v =  m_palram[0x200 + i] & 0xff;
		double bf = y1+1.772*(u - 128);
		double gf = y1-0.334*(u - 128) - 0.714 * (v - 128);
		double rf = y1+1.772*(v - 128);
		// clamp to 0-255 range
		rf = std::min(rf,255.0);
		rf = std::max(rf,0.0);
		gf = std::min(gf,255.0);
		gf = std::max(gf,0.0);
		bf = std::min(bf,255.0);
		bf = std::max(bf,0.0);

		uint8_t r = (uint8_t)rf;
		uint8_t g = (uint8_t)gf;
		uint8_t b = (uint8_t)bf;

		m_palette->set_pen_color(i, r, g, b);
	}

	if (1) // one of the m_unkregs[0x10] bits almost certainly would enable / disable this
	{
		render_tile_layer(screen, bitmap, cliprect, 0);
	}

	draw_sprites(screen, bitmap, cliprect, 0);

	if (m_unkregs[0x10] & 0x10) // definitely looks like layer enable
	{
		render_tile_layer(screen, bitmap, cliprect, 1);
	}

	draw_sprites(screen, bitmap, cliprect, 1);

	if (m_unkregs[0x10] & 0x20) // this layer is buggy and not currently drawn
	{
		int base = (m_unkregs[0x56] << 8);
		render_text_tile_layer(screen, bitmap, cliprect, base);
	}

	return 0;
}




void trkfldch_state::trkfldch_map(address_map &map)
{
	map(0x000000, 0x003fff).ram().share("mainram");

	map(0x006800, 0x006cff).ram().share("spriteram");

	map(0x007000, 0x0072ff).ram().share("palram");

	// 7800 - 78xx look like registers?
	map(0x007800, 0x0078ff).rw(FUNC(trkfldch_state::unkregs_r), FUNC(trkfldch_state::unkregs_w));

	map(0x008000, 0x3fffff).rom().region("maincpu", 0x000000); // good for code mapped at 008000 and 050000 at least
}

void trkfldch_state::vectors_map(address_map &map)
{
	map(0x00, 0x1f).r(FUNC(trkfldch_state::read_vector));
}

READ8_MEMBER(trkfldch_state::read_vector)
{
	uint8_t *rom = memregion("maincpu")->base();

	/* what appears to be a table of vectors apepars at the START of ROM, maybe this gets copied to RAM, maybe used directly?
	00 : (invalid)
	02 : (invalid)
	04 : 0xA2C6  (dummy)
	06 : 0xA334  (real function - vbl?)
	08 : 0xA300  (dummy)
	0a : 0xA2E0  (dummy)
	0c : 0xA2B9  (dummy)
	0e : 0xA2ED  (dummy)
	10 : 0xA2D3  (dummy)
	12 : 0xA327  (dummy)
	14 : 0xA30D  (real function)
	16 : 0x6000  (points at ram? or some internal ROM? we have nothing mapped here, not cleared as RAM either)
	18 : 0xA31A  (dummy)
	1a : 0xA2AC  (dummy)
	1c : 0xA341  (boot vector)
	1e : (invalid)
	*/

	logerror("reading vector offset %02x\n", offset);

	if (offset == 0x0b)
	{   // NMI
		return rom[m_which_vector+1];
	}
	else if (offset == 0x0a)
	{   // NMI
		return rom[m_which_vector];
	}

	// boot vector
	return rom[offset];
}


TIMER_DEVICE_CALLBACK_MEMBER(trkfldch_state::scanline)
{
	int scanline = param;

	if (scanline == 200)
	{
		m_which_vector = 0x06;
		m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else if (scanline == 201)
	{
		m_which_vector = 0x06;
		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}

	if (scanline == 20)
	{
		m_which_vector = 0x14;
		m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);
	}
	else if (scanline == 21)
	{
		m_which_vector = 0x14;
		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE);
	}

}

static INPUT_PORTS_START( trkfldch )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( my1stddr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("O") // selects / forward
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // directions correct based on 'letters' minigame
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("X") // goes back
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 48,49, 32,33, 16,17, 0, 1 },
	{ 8,10,12, 14, 0,2,4,6  },
	{ STEP8(0,64) },
	512,
};

static const gfx_layout tiles8x16x8_layout =
{
	8,16,
	RGN_FRAC(1,1),
	8,
	{ 48,49, 32,33, 16,17, 0, 1 },
	{ 8,10,12, 14, 0,2,4,6  },
	{ STEP16(0,64) },
	1024,
};


static const gfx_layout tiles16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 96, 97, 64, 65, 32, 33, 0, 1  },
	{ 8,10,12,14, 0,2,4,6, 24,26,28,30, 16,18,20,22 },
	{ STEP16(0,128) },
	128*16,
};


// TODO: if we're going to use gfxdecode then allocate this manually with the correct number of tiles
//  might be we have to use manual drawing tho, the base offset is already strange
static const gfx_layout tiles8x8x6_layout =
{
	8,8,
	0x5500*4,
	6,
	{ 32, 33, 16, 17, 0, 1  },
	{ 8,10,12,14, 0,2,4,6 },
	{ STEP8(0,48) },
	48*8,
};


static const gfx_layout tiles16x16x6_layout =
{
	16,16,
	0x5500,
	6,
	{ 64, 65, 32, 33, 0, 1  },
	{ 8,10,12,14, 0,2,4,6, 24,26,28,30, 16,18,20,22 },
	{ STEP16(0,96) },
	96*16,
};


static GFXDECODE_START( gfx_trkfldch )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x8x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0, tiles16x16x8_layout, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x40, tiles8x8x6_layout, 0, 4 )
	GFXDECODE_ENTRY( "maincpu", 0x40, tiles16x16x6_layout, 0, 4 )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x16x8_layout, 0, 1 )
GFXDECODE_END

/*

7800 / 7801 seem to be IRQ related

7800 : 0001 - ? (there is no irq 0x00)
       0002 - ? (there is no irq 0x02)
       0004 used in irq 0x04
       0008 used in irq 0x06
       0010 used in irq 0x08
       0020 used in irq 0x0a
       0x40 used in irq 0x0c
       0x80 used in irq 0x0e (and by code accessing other ports in the main execution?!)

7801 : 0001 used in irq 0x10
     : 0002 used in irq 0x12
     : 0004 used in irq 0x14
     : 0008 - ? (there is no irq 0x016, it points to unknown area? and we have no code touching this bit)
     : 0010 used in irq 0x18
     : 0020 used in irq 0x1a and 0x06?! (used with OR instead of EOR in 0x06, force IRQ?)
     : 0x40 - ? (there is no irq 0x1c - it's the boot vector)
     : 0x80 - ? (there is no irq 0x1e)


42/43 = y scroll on one layer?

26/27 = x xcroll (top racer on track field)
28/29 = x scroll (bottom racer on track field)

2a/2b = ?? throwing event (paired with below)
3a/3b = ?? throwing event (paired with below)

54 = 18 (layer base?) (tiles are at 1800 trkfldch)
55 = 22 (layer base?) (tiles are at 2200 trkfldch)
56 = 2c (layer base?) (tiles are at 2c00 trkfldch)


*/

READ8_MEMBER(trkfldch_state::unkregs_r)
{
	uint8_t ret = m_unkregs[offset];

	switch (offset)
	{
	case 0x00: // IRQ status?, see above
		ret = machine().rand();
		logerror("%s: unkregs_r (IRQ state?) %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x01: // IRQ status?, see above
		ret = machine().rand();
		logerror("%s: unkregs_r (IRQ state?) %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x02: // ends up being read as a side effect of reading a 16-bit word at 0x1, but also directly too?
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x03: // ends up being read as a side effect of reading a 16-bit word at 0x2, any other purpose?
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	case 0x04:
		ret = 0xff;
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x05: // only read as a side effect of reading port 0x4 in 16-bit mode?
		ret = 0xff;
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x06:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	case 0x42:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x43:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x44:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;



	case 0x54: // tilebase 1
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x55: // tilebase 2
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x56: // tilebase 3
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;




	case 0x70: // read in irq (inputs?)
		ret = ioport("IN0")->read();
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x71:
		ret = ioport("IN1")->read();
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x73:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x74:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x75:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x76:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x77:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x7f:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0x80: // only read as a side-effect of reading 0x7f?
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;



	case 0xb6:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;

	case 0xb7:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;


	default:
		logerror("%s: unkregs_r %04x (returning %02x)\n", machine().describe_context(), offset, ret);
		break;
	}
	return ret;
}

WRITE8_MEMBER(trkfldch_state::unkregs_w)
{
	m_unkregs[offset] = data;

	switch (offset)
	{
	case 0x00: // IRQ ack/force?, see above
		logerror("%s: unkregs_w (IRQ ack/force?) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x01: // IRQ maybe status, see above
		logerror("%s: unkregs_w (IRQ ack/force?) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x02: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x03: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x04: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x05: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;





	case 0x10: // gfxmode select (4bpp / 8bpp) and layer enables
		logerror("%s: unkregs_w (enable, bpp select) %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x11: // tilegfxbank 1
		logerror("%s: unkregs_w %04x %02x (tilegfxbank 1)\n", machine().describe_context(), offset, data);
		break;

	case 0x12: // 00 - startup (probably more tilegfxbank 1 bits)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x13: // tilegfxbank 2
		logerror("%s: unkregs_w %04x %02x (tilegfxbank 2)\n", machine().describe_context(), offset, data);
		break;

	case 0x14: // 00 - startup (probably more tilegfxbank 2 bits)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x15: // spritegfxbank
		logerror("%s: unkregs_w  %04x %02x (spritegfxbank)\n", machine().describe_context(), offset, data);
		break;

	case 0x16: // 00 (probably more spritegfxbank bits)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x17: // gfxbank for weird layer
		logerror("%s: unkregs_w %04x %02x (weird gfx bank)\n", machine().describe_context(), offset, data);
		break;

	case 0x18: // more gfxbank for weird layer?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	// unknowns? another unknown layer?
	case 0x19:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x1a:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;




	case 0x20: // rarely
		logerror("%s: unkregs_w %04x %02x (window 0 top?)\n", machine().describe_context(), offset, data);  // trkfldch possible scroll window 0 top (0f)
		break;

	case 0x21: // rarely
		logerror("%s: unkregs_w %04x %02x (window 0 bottom?)\n", machine().describe_context(), offset, data); // trkfldch possible scroll window 0 bottom (1f)
		break;

	case 0x22: // rarely
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data); // 1f
		break;

	case 0x23: // after a long time
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data); // 1e
		break;

	case 0x24: // rarely
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data); // 00
		break;

	case 0x25: // rarely
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data); // 00
		break;

	case 0x26:
		logerror("%s: unkregs_w %04x %02x (x scroll 2 low) upper\n", machine().describe_context(), offset, data); // trkfldch running (split screen)
		break;

	case 0x27:
		logerror("%s: unkregs_w %04x %02x (x scroll 2 high) upper\n", machine().describe_context(), offset, data); // trkfldch running (split screen)
		break;

	case 0x28:
		logerror("%s: unkregs_w %04x %02x (x scroll 2 low) lower\n", machine().describe_context(), offset, data); // trkfldch running (split screen)
		break;

	case 0x29:
		logerror("%s: unkregs_w %04x %02x (x scroll 2 high) lower\n", machine().describe_context(), offset, data); // trkfldch running (split screen)
		break;

	case 0x2a:
		logerror("%s: unkregs_w %04x %02x (x scroll 1 low)\n", machine().describe_context(), offset, data); // trkfldch jav
		break;

	case 0x2b:
		logerror("%s: unkregs_w %04x %02x (x scroll 1 high)\n", machine().describe_context(), offset, data); // trkfldch jav
		break;




	case 0x30:
		logerror("%s: unkregs_w %04x %02x (y scroll 1 low)\n", machine().describe_context(), offset, data); // trkfldch jav, hammer
		break;

	case 0x31:
		logerror("%s: unkregs_w %04x %02x (y scroll 1 high)\n", machine().describe_context(), offset, data); // trkfldch jav, hammer
		break;

	case 0x32: // rarely
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data); // 19 / 00
		break;

	case 0x33: // rarely
		logerror("%s: unkregs_w %04x %02x (window 1 top?)\n", machine().describe_context(), offset, data); // 1e / 04 possible scroll window 1 top
		break;

	case 0x34: // rarely
		logerror("%s: unkregs_w %04x %02x (window 1 bottom?)\n", machine().describe_context(), offset, data); // 1e / 19 possible scroll window 1 bottom
		break;


	// gap


	case 0x36: // rarely
		logerror("%s: unkregs_w %04x %02x (window 1 left)\n", machine().describe_context(), offset, data); // 25  possible scroll window 1 left
		break;

	case 0x37: // rarely
		logerror("%s: unkregs_w %04x %02x (window 1 right)\n", machine().describe_context(), offset, data); // 29  possible scroll window 1 right
		break;

	case 0x3a:
		logerror("%s: unkregs_w %04x %02x (x scroll 2 low)\n", machine().describe_context(), offset, data);  // trkfldch jav
		break;

	case 0x3b:
		logerror("%s: unkregs_w %04x %02x (x scroll 2 high)\n", machine().describe_context(), offset, data);  // trkfldch jav
		break;




	case 0x42:
		logerror("%s: unkregs_w %04x %02x (y scroll 2 low)\n", machine().describe_context(), offset, data); // my1stddr text scroller on right
		break;

	case 0x43:
		logerror("%s: unkregs_w %04x %02x (y scroll 2 high)\n", machine().describe_context(), offset, data); // my1stddr text scroller on right
		break;




	case 0x54: // tilebase 1
		logerror("%s: unkregs_w %04x %02x (tilebase 1)\n", machine().describe_context(), offset, data);
		break;

	case 0x55: // tilebase 2
		logerror("%s: unkregs_w %04x %02x (tilebase 2)\n", machine().describe_context(), offset, data);
		break;

	case 0x56: // tilebase 3
		logerror("%s: unkregs_w %04x %02x (weird layer base)\n", machine().describe_context(), offset, data);
		break;



	case 0x60: // sprite list location (dma source?)
		logerror("%s: unkregs_w %04x %02x (dma source low )\n", machine().describe_context(), offset, data);
		break;

	case 0x61: // sprite list location (dma source?)
		logerror("%s: unkregs_w %04x %02x (dma source med )\n", machine().describe_context(), offset, data);
		break;

	case 0x62:
		logerror("%s: unkregs_w %04x %02x (dma source high)\n", machine().describe_context(), offset, data);
		break;

	case 0x63:
		logerror("%s: unkregs_w %04x %02x (dma dest low )\n", machine().describe_context(), offset, data);
		break;

	case 0x64:
		logerror("%s: unkregs_w %04x %02x (dma dest high)\n", machine().describe_context(), offset, data);
		break;

	case 0x65:
		logerror("%s: unkregs_w %04x %02x (dma length low ) (and trigger)\n", machine().describe_context(), offset, data);
		{
			address_space &mem = m_maincpu->space(AS_PROGRAM);
			uint16_t dmalength = (m_unkregs[0x66] << 8) | m_unkregs[0x65];
			uint32_t dmasource = (m_unkregs[0x62] << 16) | (m_unkregs[0x61] << 8) | m_unkregs[0x60];
			uint16_t dmadest = (m_unkregs[0x64] << 8) | m_unkregs[0x63];

			//if (dmadest != 0x6800)
			logerror("%s: performing dma src: %06x dst %04x len %04x and extra params %02x %02x %02x %02x %02x %02x\n", machine().describe_context(), dmasource, dmadest, dmalength, m_unkregs[0x67], m_unkregs[0x68], m_unkregs[0x69], m_unkregs[0x6b], m_unkregs[0x6c], m_unkregs[0x6d]);

			int writeoffset = 0;
			int writedo = m_unkregs[0x6d];

			int readoffset = 0;
			int readdo = m_unkregs[0x69];

			if ((m_unkregs[0x68] != 0x00) || (m_unkregs[0x6c] != 0x00))
			{
				fatalerror("unhandled dma params\n");
			}

			for (uint32_t j = 0; j < dmalength; j++)
			{
				uint8_t byte = mem.read_byte(dmasource+readoffset);
				readdo--;
				if (readdo < 0)
				{
					readdo = m_unkregs[0x69];
					readoffset += m_unkregs[0x67];
				}
				else
				{
					readoffset++;
				}

				mem.write_byte(dmadest+writeoffset, byte);
				writedo--;
				if (writedo < 0)
				{
					writedo = m_unkregs[0x6d];
					writeoffset += m_unkregs[0x6b];
				}
				else
				{
					writeoffset++;
				}

			}
		}
		break;

	case 0x66:
		logerror("%s: unkregs_w %04x %02x (dma length high)\n", machine().describe_context(), offset, data);
		break;

	case 0x67: // after a long time
		logerror("%s: unkregs_w %04x %02x (dma source read skip size)\n", machine().describe_context(), offset, data);
		break;

	case 0x68: // rarely (my1stddr)
		logerror("%s: unkregs_w %04x %02x (dma source unknown)\n", machine().describe_context(), offset, data);
		break;

	case 0x69: // after a long time
		logerror("%s: unkregs_w %04x %02x (dma source read group size)\n", machine().describe_context(), offset, data);
		break;

	case 0x6b: // after a long time
		logerror("%s: unkregs_w %04x %02x (dma dest write skip size)\n", machine().describe_context(), offset, data);
		break;

	case 0x6c: // rarely (my1stddr)
		logerror("%s: unkregs_w %04x %02x (dma dest unknown)\n", machine().describe_context(), offset, data);
		break;

	case 0x6d: // after a long time
		logerror("%s: unkregs_w %04x %02x (dma dest write group size)\n", machine().describe_context(), offset, data);
		break;



	// 7x = I/O area?


	case 0x71: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x72: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x73: // some kind of serial device?
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x74: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x75: // some kind of serial device? (used with 73?)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x76: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x77: // every second or so
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x78: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x79: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x7a: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x7f: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	case 0x81: // startup (my1stddr)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x82: // startup (my1stddr)
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x83:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0x84:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	case 0xb5: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;

	case 0xb6: // significant data transfer shortly after boot, seems to clock writes with 0073 writing  d0 / c0? (then writes 2 bytes here)
			   // is this sending song patterns to another CPU?

		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		m_unkdata[m_unkdata_addr] = data;

		m_unkdata_addr++;
		m_unkdata_addr &= 0xfffff;
		break;



	case 0xca: // startup
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;


	default:
		logerror("%s: unkregs_w %04x %02x\n", machine().describe_context(), offset, data);
		break;
	}

}

void trkfldch_state::machine_start()
{
	save_item(NAME(m_unkdata_addr));
	save_item(NAME(m_unkdata));

	for (int i = 0; i < 256; i++)
	{
		m_palette->set_pen_color(i, machine().rand(), machine().rand(), machine().rand());
	}

}

void trkfldch_state::machine_reset()
{
	m_which_vector = 0x06;

	for (int i = 0; i < 0x100; i++)
		m_unkregs[i] = 0x00;

	for (int i = 0; i < 0x100000; i++)
		m_unkdata[i] = 0;

	m_unkdata_addr = 0;

	// the game code doesn't set the DMA step / skip params to default values until after it's used them with other values, so assume they reset to these
	m_unkregs[0x67] = 0x01;
	m_unkregs[0x68] = 0x00;
	m_unkregs[0x69] = 0x00;

	m_unkregs[0x6b] = 0x01;
	m_unkregs[0x6c] = 0x00;
	m_unkregs[0x6d] = 0x00;

	// maybe, these get reset to this value after actual use in trkfield
	m_unkregs[0x36] = 0x00;
	m_unkregs[0x37] = 0x28;

	// trkfld sets these to 1e/1e on javelin, but leaves then uninitizlied before that
	// it also sets them to 00 / 1e on 'hurdle the hole' (window use)
	// it also sets them to 00 / 1e on 'hammer throw' (reason unclear)
	// high jump sets it to 00 / 16 on height select screen
	m_unkregs[0x33] = 0xde;
	m_unkregs[0x34] = 0xad;
}

void trkfldch_state::trkfldch(machine_config &config)
{
	/* basic machine hardware */
	G65816(config, m_maincpu, 20000000);
	//m_maincpu->set_addrmap(AS_DATA, &trkfldch_state::mem_map);
	m_maincpu->set_addrmap(AS_PROGRAM, &trkfldch_state::trkfldch_map);
	m_maincpu->set_addrmap(g65816_device::AS_VECTORS, &trkfldch_state::vectors_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(trkfldch_state::scanline), "screen", 0, 1);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(trkfldch_state::screen_update_trkfldch));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_trkfldch);

	PALETTE(config, m_palette, palette_device::BLACK, 256);
}

ROM_START( trkfldch )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "trackandfield.bin", 0x000000, 0x400000,  CRC(f4f1959d) SHA1(344dbfe8df1897adf77da6e5ca0435c4d47d6842) )
ROM_END

ROM_START( my1stddr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "myfirstddr.bin", 0x000000, 0x400000, CRC(2ef57bfc) SHA1(9feea5adb9de8fe17e915f3a037e8ddd70e58ae7) )
ROM_END


CONS( 2007, trkfldch,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",             "Track & Field Challenge", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 2006, my1stddr,  0,          0,  trkfldch, my1stddr,trkfldch_state,      empty_init,    "Konami",             "My First Dance Dance Revolution (US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Japan version has different songs

