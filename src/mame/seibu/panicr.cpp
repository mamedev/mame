// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
Panic Road
----------

TODO:
 - collisions don't always work, you can hit the ball out of the playfield quite easily if you know how, hence MACHINE_NOT_WORKING
 - are priorities with sprites 100%, sprite-sprite priorities are ugly in many places, maybe the SEI0010BU are 3 sprite chips?

--

Panic Road (JPN Ver.)
(c)1986 Taito / Seibu
SEI-8611M (M6100219A)

OSC  : 14.31818MHz,12.0000MHz,16.0000MHz
CPU  : V20 (Sony CXQ70116D-8) @ 8.000MHz [16/2]
       Toshiba T5182 @ 3.579545 [14.31818/4]
Sound: YM2151 @ 3.579545 [14.31818/4]
    VSync 60Hz
    HSync 15.32kHz
Other:
    SEI0010BU(TC17G005AN-0025) x3
    SEI0021BU(TC17G008AN-0022)
    Toshiba(TC17G008AN-0024)
    SEI0030BU(TC17G005AN-0026)
    SEI0050BU(MA640 00)
    SEI0040BU(TC15G008AN-0048) @ 6.00MHz [12/2]


13F.BIN      [4e6b3c04]
15F.BIN      [d735b572]

22D.BIN      [eb1a46e1]

5D.BIN       [f3466906]
7D.BIN       [8032c1e9]

2A.BIN       [3ac0e5b4]
2B.BIN       [567d327b]
2C.BIN       [cd77ec79]
2D.BIN       [218d2c3e]

2J.BIN       [80f05923]
2K.BIN       [35f07bca]

1.19N        [674131b9]
2.19M        [3d48b0b5]

A.15C        [c75772bc] 82s129
B.14C        [145d1e0d]  |
C.13C        [11c11bbd]  |
D.9B         [f99cac4b] /

8A.BPR       [908684a6] 63s281
10J.BPR      [1dd80ee1]  |
10L.BPR      [f3f29695]  |
12D.BPR      [0df8aa3c] /

*/

#include "emu.h"
#include "t5182.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class panicr_state : public driver_device
{
public:
	panicr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_t5182(*this, "t5182"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_textram(*this, "textram"),
		m_spritebank(*this, "spritebank"),
		m_tilerom(*this, "tilerom"),
		m_attrrom(*this, "attrrom")
	{ }

	void panicr(machine_config &config);

	void init_panicr();

private:
	void textram_w(offs_t offset, uint8_t data);
	uint8_t collision_r(offs_t offset);
	void scrollx_lo_w(uint8_t data);
	void scrollx_hi_w(uint8_t data);
	void output_w(uint8_t data);
	uint8_t t5182shared_r(offs_t offset);
	void t5182shared_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bgtile_info);
	TILE_GET_INFO_MEMBER(get_infotile_info_2);
	TILE_GET_INFO_MEMBER(get_txttile_info);

	virtual void video_start() override ATTR_COLD;
	void panicr_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void panicr_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<t5182_device> m_t5182;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_textram;
	required_shared_ptr<uint8_t> m_spritebank;

	required_region_ptr<uint8_t> m_tilerom;
	required_region_ptr<uint8_t> m_attrrom;

	tilemap_t *m_bgtilemap = nullptr;
	tilemap_t *m_infotilemap_2 = nullptr;
	tilemap_t *m_txttilemap = nullptr;

	int m_scrollx = 0;
	std::unique_ptr<bitmap_ind16> m_temprender;
	std::unique_ptr<bitmap_ind16> m_tempbitmap_1;
	rectangle m_tempbitmap_clip;

};


//#define TC15_CLOCK      XTAL(12'000'000)


/***************************************************************************

  Video

***************************************************************************/

void panicr_state::panicr_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x100; i++)
	{
		int const r = pal4bit(color_prom[i + 0x000]);
		int const g = pal4bit(color_prom[i + 0x100]);
		int const b = pal4bit(color_prom[i + 0x200]);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x300;

	// txt lookup table
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x40) ? 0 : ((color_prom[i] & 0x3f) | 0x80);
		palette.set_pen_indirect(i, ctabentry);
	}

	// tile lookup table
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i + 0x100] & 0x3f) | 0x00;

		palette.set_pen_indirect(((i & 0x0f) + ((i & 0xf0) << 1)) + 0x200, ctabentry);
		palette.set_pen_indirect(((i & 0x0f) + ((i & 0xf0) << 1)) + 0x210, ctabentry);
	}

	// sprite lookup table
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i + 0x200] & 0x40) ? 0 : ((color_prom[i + 0x200] & 0x3f) | 0x40);

		palette.set_pen_indirect(i + 0x100, ctabentry);
	}
}


TILE_GET_INFO_MEMBER(panicr_state::get_bgtile_info)
{
	int code = m_tilerom[tile_index];
	int const attr = m_attrrom[tile_index];
	code += ((attr & 7) << 8);
	tileinfo.set(1,
		code,
		(attr & 0xf0) >> 4,
		0);
}


TILE_GET_INFO_MEMBER(panicr_state::get_infotile_info_2)
{
	int code = m_tilerom[tile_index];
	int const attr = m_attrrom[tile_index];
	code += ((attr & 7) << 8);
	tileinfo.set(3,
		code,
		0,
		0);
}


TILE_GET_INFO_MEMBER(panicr_state::get_txttile_info)
{
	int const code = m_textram[tile_index * 4];
	int const attr = m_textram[tile_index * 4 + 2];
	int const color = attr & 0x07;

	tileinfo.group = color;

	tileinfo.set(0,
		code + ((attr & 8) << 5),
		color,
		0);
}


void panicr_state::video_start()
{
	m_bgtilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(panicr_state::get_bgtile_info)), TILEMAP_SCAN_ROWS, 16,16, 1024,16);
	m_infotilemap_2 = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(panicr_state::get_infotile_info_2)), TILEMAP_SCAN_ROWS, 16,16, 1024,16);

	m_txttilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(panicr_state::get_txttile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);
	m_txttilemap->configure_groups(*m_gfxdecode->gfx(0), 0);

	save_item(NAME(m_scrollx));
}

void panicr_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	// ssss ssss | Fx-- cccc | yyyy yyyy | xxxx xxxx

	for (int offs = m_spriteram.bytes() - 16; offs >= 0; offs -= 16)
	{
		bool const flipx = false;
		bool const flipy = BIT(m_spriteram[offs + 1], 7);
		int const y = m_spriteram[offs + 2];
		int x = m_spriteram[offs + 3];
		if (BIT(m_spriteram[offs + 1], 6)) x -= 0x100;

		if (BIT(m_spriteram[offs + 1], 5))
		{
			// often set
		}

		if (BIT(m_spriteram[offs + 1], 4))
		{
			popmessage("(BIT(spriteram[offs + 1], 4)) %02x\n", BIT(m_spriteram[offs + 1], 4));
		}

		uint32_t const color = m_spriteram[offs + 1] & 0x0f;
		uint32_t const sprite = m_spriteram[offs + 0] | (*m_spritebank << 8);

		m_gfxdecode->gfx(2)->transmask(bitmap,cliprect,
				sprite,
				color, flipx, flipy, x, y,
				m_palette->transpen_mask(*m_gfxdecode->gfx(2), color, 0));
	}
}

uint32_t panicr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bgtilemap->set_scrollx(0, m_scrollx);
	m_bgtilemap->draw(screen, *m_temprender, m_tempbitmap_clip, 0,0);

//  m_infotilemap_2->set_scrollx(0, m_scrollx);
//  m_infotilemap_2->draw(screen, *m_temprender, m_tempbitmap_clip, 0,0);

	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *const srcline = &m_temprender->pix(y);
		uint16_t *const dstline = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t const dat = srcline[x];

			dstline[x] = ((dat & 0x00f) | ((dat & 0x1e0)>>0)) + 0x200;
		}
	}

	draw_sprites(bitmap,cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *const srcline = &m_temprender->pix(y);
		uint16_t *const dstline = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t const dat = srcline[x];
			if (dat & 0x10)
				dstline[x] = ((dat & 0x00f) | ((dat & 0x1e0)>>0)) + 0x200;
		}
	}

	m_txttilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}


/***************************************************************************

  I/O / Memory

***************************************************************************/

void panicr_state::textram_w(offs_t offset, uint8_t data)
{
	m_textram[offset] = data;
	if (BIT(~offset, 0))
		m_txttilemap->mark_tile_dirty(offset >> 2);
}


uint8_t panicr_state::collision_r(offs_t offset)
{
	// re-render the collision data here
	// collisions are based on 2 bits from the tile data, relative to a page of tiles
	//
	// the 2 collision bits represent
	// solid areas of the playfield
	// areas where flippers affect the ball
	//
	// there is a 3rd additional bit that is used as priority, we're not concerned about that here

	m_infotilemap_2->set_scrollx(0, m_scrollx & 0xffff);
	m_infotilemap_2->draw(*m_screen, *m_tempbitmap_1, m_tempbitmap_clip, 0,0);

	int actual_column = offset & 0x3f;
	int const actual_line = offset >> 6;

	actual_column = actual_column * 4;

	actual_column -= m_scrollx;
	actual_column &= 0xff;

	uint8_t ret = 0;
	uint16_t const *const srcline = &m_tempbitmap_1->pix(actual_line);

	ret |= (srcline[(actual_column + 0) & 0xff] & 3) << 6;
	ret |= (srcline[(actual_column + 1) & 0xff] & 3) << 4;
	ret |= (srcline[(actual_column + 2) & 0xff] & 3) << 2;
	ret |= (srcline[(actual_column + 3) & 0xff] & 3) << 0;

	if (!machine().side_effects_disabled())
		logerror("%06x: (scroll x upper bits is %04x (full %04x)) read %d %d\n", m_maincpu->pc(), (m_scrollx&0xff00)>>8, m_scrollx,  actual_line, actual_column);

	return ret;
}


void panicr_state::scrollx_lo_w(uint8_t data)
{
	logerror("scrollx_lo_w %02x\n", data);
	m_scrollx = (m_scrollx & 0xff00) | (data << 1 & 0xfe) | (data >> 7 & 0x01);
}

void panicr_state::scrollx_hi_w(uint8_t data)
{
	logerror("scrollx_hi_w %02x\n", data);
	m_scrollx = (m_scrollx & 0xff) | ((data &0xf0) << 4) | ((data & 0x0f) << 12);
}

void panicr_state::output_w(uint8_t data)
{
	// d6, d7: play counter? (it only triggers on 1st coin)
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7));

	logerror("output_w %02x\n", data);

	// other bits: ?
}

uint8_t panicr_state::t5182shared_r(offs_t offset)
{
	if ((offset & 1) == 0)
		return m_t5182->sharedram_r(offset/2);
	else
		return 0;
}

void panicr_state::t5182shared_w(offs_t offset, uint8_t data)
{
	if ((offset & 1) == 0)
		m_t5182->sharedram_w(offset/2, data);
}


void panicr_state::panicr_map(address_map &map)
{
	map(0x00000, 0x01fff).ram().share(m_mainram);
	map(0x02000, 0x03cff).ram().share(m_spriteram); // how big is sprite ram, some places definitely have sprites at 3000+
	map(0x03d00, 0x03fff).ram();
	map(0x08000, 0x0bfff).r(FUNC(panicr_state::collision_r));
	map(0x0c000, 0x0cfff).ram().w(FUNC(panicr_state::textram_w)).share(m_textram);
	map(0x0d000, 0x0d000).w(m_t5182, FUNC(t5182_device::sound_irq_w));
	map(0x0d002, 0x0d002).w(m_t5182, FUNC(t5182_device::sharedram_semaphore_main_acquire_w));
	map(0x0d004, 0x0d004).r(m_t5182, FUNC(t5182_device::sharedram_semaphore_snd_r));
	map(0x0d006, 0x0d006).w(m_t5182, FUNC(t5182_device::sharedram_semaphore_main_release_w));
	map(0x0d200, 0x0d2ff).rw(FUNC(panicr_state::t5182shared_r), FUNC(panicr_state::t5182shared_w));
	map(0x0d400, 0x0d400).portr("P1");
	map(0x0d402, 0x0d402).portr("P2");
	map(0x0d404, 0x0d404).portr("START");
	map(0x0d406, 0x0d406).portr("DSW1");
	map(0x0d407, 0x0d407).portr("DSW2");
	map(0x0d802, 0x0d802).w(FUNC(panicr_state::scrollx_hi_w));
	map(0x0d804, 0x0d804).w(FUNC(panicr_state::scrollx_lo_w));
	map(0x0d80a, 0x0d80a).w(FUNC(panicr_state::output_w));
	map(0x0d80c, 0x0d80c).writeonly().share(m_spritebank);
	map(0x0d818, 0x0d818).nopw(); // watchdog?
	map(0xf0000, 0xfffff).rom();
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( panicr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Left Flipper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Left Shake")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Right Shake")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Left Flipper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Right Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Left Shake")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Right Shake")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Bonus Points" )              PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "5k 10k" )
	PORT_DIPSETTING(    0x10, "10k 20k" )
	PORT_DIPSETTING(    0x08, "20k 40k" )
	PORT_DIPSETTING(    0x00, "50k 100k" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout bgtilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	5,
	{  /* priority bit -> */ RGN_FRAC(1,4)+0 , /* colour bits -> */ RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+4, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+4},
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*16
};


static const gfx_layout infotilelayout_2 =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ /* collision bits -> */ RGN_FRAC(1,4)+4, 4  },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*16
};




static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 4*8+0, 4*8+1, 4*8+2,  4*8+3,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 12*8+0, 12*8+1, 12*8+2, 12*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 16*8, 17*8, 18*8, 19*8,
		32*8, 33*8, 34*8, 35*8, 48*8, 49*8, 50*8, 51*8 },
	32*16
};

static GFXDECODE_START( gfx_panicr )
	GFXDECODE_ENTRY( "chars",   0, charlayout,       0x000,  8 )
	GFXDECODE_ENTRY( "tiles",   0, bgtilelayout,     0x200, 32 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     0x100, 16 )
	GFXDECODE_ENTRY( "tiles",   0, infotilelayout_2, 0x100, 16 ) // palette is just to make it viewable with F4

GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(panicr_state::scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xc4/4); // V20

	if(scanline == 0) // <unknown>
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xc8/4); // V20
}

void panicr_state::panicr(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(16'000'000);
	constexpr XTAL SOUND_CLOCK = XTAL(14'318'181);

	V20(config, m_maincpu, MASTER_CLOCK/2); // Sony 8623h9 CXQ70116D-8 (V20 compatible)
	m_maincpu->set_addrmap(AS_PROGRAM, &panicr_state::panicr_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(panicr_state::scanline), "screen", 0, 1);

	T5182(config, m_t5182, SOUND_CLOCK/4);
	m_t5182->ym_read_callback().set("ymsnd", FUNC(ym2151_device::read));
	m_t5182->ym_write_callback().set("ymsnd", FUNC(ym2151_device::write));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(32*8, 32*8);
//  m_screen->set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(panicr_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_panicr);
	PALETTE(config, m_palette, FUNC(panicr_state::panicr_palette), 256 * 4, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", SOUND_CLOCK/4)); // 3.579545 MHz
	ymsnd.irq_handler().set(m_t5182, FUNC(t5182_device::ym2151_irq_handler));
	ymsnd.add_route(0, "mono", 1.0);
	ymsnd.add_route(1, "mono", 1.0);
}


ROM_START( panicr )
	ROM_REGION( 0x100000, "maincpu", 0 ) // v20 main cpu
	ROM_LOAD16_BYTE("2.19m",   0x0f0000, 0x08000, CRC(3d48b0b5) SHA1(a6e8b38971a8964af463c16f32bb7dbd301dd314) )
	ROM_LOAD16_BYTE("1.19n",   0x0f0001, 0x08000, CRC(674131b9) SHA1(63499cd5ad39e79e70f3ba7060680f0aa133f095) )

	ROM_REGION( 0x8000, "t5182:external", 0 ) // Toshiba T5182 external ROM
	ROM_LOAD( "22d.bin",   0x0000, 0x8000, CRC(eb1a46e1) SHA1(278859ae4bca9f421247e646d789fa1206dcd8fc) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "13f.bin", 0x000000, 0x2000, CRC(4e6b3c04) SHA1(f388969d5d822df0eaa4d8300cbf9cee47468360) )
	ROM_LOAD( "15f.bin", 0x002000, 0x2000, CRC(d735b572) SHA1(edcdb6daec97ac01a73c5010727b1694f512be71) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "2a.bin", 0x000000, 0x20000, CRC(3ac0e5b4) SHA1(96b8bdf02002ec8ce87fd47fd21f7797a79d79c9) )
	ROM_LOAD( "2b.bin", 0x020000, 0x20000, CRC(567d327b) SHA1(762b18ef1627d71074ba02b0eb270bd9a01ac0d8) )
	ROM_LOAD( "2c.bin", 0x040000, 0x20000, CRC(cd77ec79) SHA1(94b61b7d77c016ae274eddbb1e66e755f312e11d) )
	ROM_LOAD( "2d.bin", 0x060000, 0x20000, CRC(218d2c3e) SHA1(9503b3b67e71dc63448aed7815845b844e240afe) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "2j.bin", 0x000000, 0x20000, CRC(80f05923) SHA1(5c886446fd77d3c39cb4fa43ea4beb8c89d20636) )
	ROM_LOAD( "2k.bin", 0x020000, 0x20000, CRC(35f07bca) SHA1(54e6f82c2e6e1373c3ac1c6138ef738e5a0be6d0) )

	ROM_REGION( 0x04000, "tilerom", 0 )
	ROM_LOAD( "5d.bin", 0x00000, 0x4000, CRC(f3466906) SHA1(42b512ba93ba7ac958402d1871c5ae015def3501) ) //tilemaps
	ROM_REGION( 0x04000, "attrrom", 0 )
	ROM_LOAD( "7d.bin", 0x00000, 0x4000, CRC(8032c1e9) SHA1(fcc8579c0117ebe9271cff31e14a30f61a9cf031) ) //attribute maps

	ROM_REGION( 0x0800,  "proms", 0 )
	ROM_LOAD( "b.14c",   0x00000, 0x100, CRC(145d1e0d) SHA1(8073fd176a1805552a5ac00ca0d9189e6e8936b1) ) // red
	ROM_LOAD( "a.15c",   0x00100, 0x100, CRC(c75772bc) SHA1(ec84052aedc1d53f9caba3232ffff17de69561b2) ) // green
	ROM_LOAD( "c.13c",   0x00200, 0x100, CRC(11c11bbd) SHA1(73663b2cf7269a62011ee067a026269ce0c15a7c) ) // blue
	ROM_LOAD( "12d.bpr", 0x00300, 0x100, CRC(0df8aa3c) SHA1(5149265d788ea4885793b0786f765524b4745f04) ) // txt lookup table
	ROM_LOAD( "8a.bpr",  0x00400, 0x100, CRC(908684a6) SHA1(82d9cb8aed576d1132615b5341c36ef51856b3a6) ) // tile lookup table
	ROM_LOAD( "10j.bpr", 0x00500, 0x100, CRC(1dd80ee1) SHA1(2d634e75666b919446e76fd35a06af27a1a89707) ) // sprite lookup table
	ROM_LOAD( "d.9b",    0x00600, 0x100, CRC(f99cac4b) SHA1(b4e6d0e0186fe186e747a9f6857b97591948c682) ) // unknown
	ROM_LOAD( "10l.bpr", 0x00700, 0x100, CRC(f3f29695) SHA1(2607e96564a5e6e9a542377a01f399ea86a36c48) ) // unknown
ROM_END

ROM_START( panicrg ) // Distributed by TV-Tuning Videospiele GMBH
	ROM_REGION( 0x100000, "maincpu", 0 ) // v20 main cpu
	ROM_LOAD16_BYTE("2g.19m",   0x0f0000, 0x08000, CRC(cf759403) SHA1(1a0911c943ecc752e46873c9a5da981745f7562d) )
	ROM_LOAD16_BYTE("1g.19n",   0x0f0001, 0x08000, CRC(06877f9b) SHA1(8b92209d6422ff2b1f3cb66bd39a3ff84e399eec) )

	ROM_REGION( 0x10000, "t5182:external", 0 ) // Toshiba T5182 external ROM
	ROM_LOAD( "22d.bin",   0x0000, 0x8000, CRC(eb1a46e1) SHA1(278859ae4bca9f421247e646d789fa1206dcd8fc) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "13f.bin", 0x000000, 0x2000, CRC(4e6b3c04) SHA1(f388969d5d822df0eaa4d8300cbf9cee47468360) )
	ROM_LOAD( "15f.bin", 0x002000, 0x2000, CRC(d735b572) SHA1(edcdb6daec97ac01a73c5010727b1694f512be71) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "2a.bin", 0x000000, 0x20000, CRC(3ac0e5b4) SHA1(96b8bdf02002ec8ce87fd47fd21f7797a79d79c9) )
	ROM_LOAD( "2b.bin", 0x020000, 0x20000, CRC(567d327b) SHA1(762b18ef1627d71074ba02b0eb270bd9a01ac0d8) )
	ROM_LOAD( "2c.bin", 0x040000, 0x20000, CRC(cd77ec79) SHA1(94b61b7d77c016ae274eddbb1e66e755f312e11d) )
	ROM_LOAD( "2d.bin", 0x060000, 0x20000, CRC(218d2c3e) SHA1(9503b3b67e71dc63448aed7815845b844e240afe) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD( "2j.bin", 0x000000, 0x20000, CRC(80f05923) SHA1(5c886446fd77d3c39cb4fa43ea4beb8c89d20636) )
	ROM_LOAD( "2k.bin", 0x020000, 0x20000, CRC(35f07bca) SHA1(54e6f82c2e6e1373c3ac1c6138ef738e5a0be6d0) )

	ROM_REGION( 0x04000, "tilerom", 0 )
	ROM_LOAD( "5d.bin", 0x00000, 0x4000, CRC(f3466906) SHA1(42b512ba93ba7ac958402d1871c5ae015def3501) ) //tilemaps
	ROM_REGION( 0x04000, "attrrom", 0 )
	ROM_LOAD( "7d.bin", 0x00000, 0x4000, CRC(8032c1e9) SHA1(fcc8579c0117ebe9271cff31e14a30f61a9cf031) ) //attribute maps

	ROM_REGION( 0x0800,  "proms", 0 )
	ROM_LOAD( "b.14c",   0x00000, 0x100, CRC(145d1e0d) SHA1(8073fd176a1805552a5ac00ca0d9189e6e8936b1) ) // red
	ROM_LOAD( "a.15c",   0x00100, 0x100, CRC(c75772bc) SHA1(ec84052aedc1d53f9caba3232ffff17de69561b2) ) // green
	ROM_LOAD( "c.13c",   0x00200, 0x100, CRC(11c11bbd) SHA1(73663b2cf7269a62011ee067a026269ce0c15a7c) ) // blue
	ROM_LOAD( "12d.bpr", 0x00300, 0x100, CRC(0df8aa3c) SHA1(5149265d788ea4885793b0786f765524b4745f04) ) // txt lookup table
	ROM_LOAD( "8a.bpr",  0x00400, 0x100, CRC(908684a6) SHA1(82d9cb8aed576d1132615b5341c36ef51856b3a6) ) // tile lookup table
	ROM_LOAD( "10j.bpr", 0x00500, 0x100, CRC(1dd80ee1) SHA1(2d634e75666b919446e76fd35a06af27a1a89707) ) // sprite lookup table
	ROM_LOAD( "d.9b",    0x00600, 0x100, CRC(f99cac4b) SHA1(b4e6d0e0186fe186e747a9f6857b97591948c682) ) // unknown
	ROM_LOAD( "10l.bpr", 0x00700, 0x100, CRC(f3f29695) SHA1(2607e96564a5e6e9a542377a01f399ea86a36c48) ) // unknown
ROM_END


void panicr_state::init_panicr()
{
	std::vector<uint8_t> buf(0x80000);

	uint8_t *rom = memregion("chars")->base();
	int size = memregion("chars")->bytes();

	// text data lines
	for (int i = 0; i < size / 2; i++)
	{
		int w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];
		w1 = bitswap<16>(w1,  9,12,7,3,  8,13,6,2, 11,14,1,5, 10,15,4,0);

		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	// text address lines
	for (int i = 0; i < size; i++)
	{
		rom[i] = buf[bitswap<24>(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6, 2,3,1,0,5,4)];
	}

	rom = memregion("tiles")->base();
	size = memregion("tiles")->bytes();

	// tiles data lines
	for (int i = 0; i < size / 4; i++)
	{
		int w1 = (rom[i + 0*size/4] << 8) + rom[i + 3*size/4];
		int w2 = (rom[i + 1*size/4] << 8) + rom[i + 2*size/4];

		w1 = bitswap<16>(w1, 14,12,11,9,   3,2,1,0, 10,15,13,8,   7,6,5,4);
		w2 = bitswap<16>(w2,  3,13,15,4, 12,2,5,11, 14,6,1,10,    8,7,9,0);

		buf[i + 0*size/4] = w1 >> 8;
		buf[i + 1*size/4] = w1 & 0xff;
		buf[i + 2*size/4] = w2 >> 8;
		buf[i + 3*size/4] = w2 & 0xff;
	}

	// tiles address lines
	for (int i = 0; i < size; i++)
	{
		rom[i] = buf[bitswap<24>(i,23,22,21,20,19,18,17,16,15,14,13,12, 5,4,3,2, 11,10,9,8,7,6, 0,1)];
	}

	rom = memregion("sprites")->base();
	size = memregion("sprites")->bytes();

	// sprites data lines
	for (int i = 0; i < size / 2; i++)
	{
		int w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];
		w1 = bitswap<16>(w1, 11,5,7,12, 4,10,13,3, 6,14,9,2, 0,15,1,8);

		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	// sprites address lines
	for (int i = 0; i < size; i++)
	{
		rom[i] = buf[i];
	}

	//rearrange  bg tilemaps a bit....
	rom = memregion("tilerom")->base();
	size = memregion("tilerom")->bytes();
	memcpy(&buf[0], rom, size);

	for (int j = 0; j < 16; j++)
	{
		for (int i = 0; i < size / 16; i += 8)
		{
			memcpy(&rom[i + (size / 16) * j], &buf[i * 16 + 8 * j], 8);
		}
	}

	rom = memregion("attrrom")->base();
	size = memregion("attrrom")->bytes();
	memcpy(&buf[0], rom, size);

	for (int j = 0; j < 16; j++)
	{
		for (int i = 0; i < size / 16; i += 8)
		{
			memcpy(&rom[i + (size / 16) * j], &buf[i * 16 + 8 * j], 8);
		}
	}

	m_tempbitmap_1 = std::make_unique<bitmap_ind16>(256, 256);
	m_temprender = std::make_unique<bitmap_ind16>(256, 256);
	m_tempbitmap_clip.set(0, 256-1, 0, 256-1);

	m_tempbitmap_1->fill(0, m_tempbitmap_clip);

}

} // anonymous namespace


GAME( 1986, panicr,  0,      panicr,  panicr, panicr_state, init_panicr, ROT270, "Seibu Kaihatsu (Taito license)", "Panic Road (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1986, panicrg, panicr, panicr,  panicr, panicr_state, init_panicr, ROT270, "Seibu Kaihatsu (Tuning license)", "Panic Road (Germany)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
