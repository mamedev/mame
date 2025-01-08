// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Alien Storm 'System 18' bootlegs

    these have extensively reworked 68000 code to handle the modified hardware

    Noteworthy features
     - no VDP
       (so no backgrounds in gallary stages, some enemies removed from first stage)
     - completely reworked background 'tilemaps'
       (regular page registers not used, instead there's a new 'row list' defining page and scroll per row)
     - less capable sound system
       (only hooked up for astormb2 set, astormbl set is using original sound, which might not be correct)

    Issues
     - title screen vanishes as soon as it appears (it's in the wrong half of 'tilemap' pages?)
       does this happen on hardware? swapping them causes issues ingame instead, currently a kludge is used to only swap one layer

     - some stuck parts in the gallery stages
       again, does this happen on hardware? fixed with same kludge as above?

     - there are 'bad' pixels near the shadows on some player sprites, but these differences in the GFX ROMs have been found on
       more than 1 PCB, does the bootleg hardware need them for some purpose?

     - the 'Action' 'Destroy' and 'Run' screens don't display properly in attract, this also seems to be related to how
       the bootleg is remapping the tilemap pages and needs checking against the bootleg hardware.

     - "Press Start" doesn't flash after inserting a credit, nor does the number of credits display
       again this should be verified against the bootleg hardware

    Currently the modified background tilemaps are not handled with the tilemap system, as some of the changes make them much more
    difficult to fit into the tilemap system.  This might be reconsidered later.

    TODO:

    tidy up + optimized video code, decide if using tilemaps is cleaner or not
    verify glitches etc. against bootleg hardware
    convert system18 / bootleg OKI sound system to devices to share with other drivers

*/

#include "emu.h"

#include "segaic16.h"
#include "sega16sp.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/rf5c68.h"
#include "sound/ymopn.h"

#include "screen.h"
#include "speaker.h"


namespace {

class segas18_astormbl_state : public sega_16bit_common_base
{
protected:
	segas18_astormbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: sega_16bit_common_base(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_sprites(*this, "sprites")
		, m_textram(*this, "textram")
		, m_tileram(*this, "tileram")
		, m_tilestripconfig(*this, "tilestripconfig")
		, m_tileenable(*this, "tileenable")
		, m_tileyscroll1(*this, "tileyscroll1")
		, m_tileyscroll0(*this, "tileyscroll0")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void video_start() override ATTR_COLD;

	virtual void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask) = 0;

	void astormbl_video(machine_config &config);
	void astormbl_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

private:
	void draw_tile(screen_device& screen, bitmap_ind16& bitmap, const rectangle &cliprect, int tilenum, int colour, int xpos, int ypos, uint8_t pri, int transpen, bool opaque);
	void draw_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle &cliprect, int layer, bool opaque, int pri0, int pri1);
	void sys16_textram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	tilemap_t *m_text_layer = nullptr;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<sega_sys16b_sprite_device> m_sprites;
	required_shared_ptr<uint16_t> m_textram;
	required_shared_ptr<uint16_t> m_tileram;
	required_shared_ptr<uint16_t> m_tilestripconfig;
	required_shared_ptr<uint16_t> m_tileenable;
	required_shared_ptr<uint16_t> m_tileyscroll1;
	required_shared_ptr<uint16_t> m_tileyscroll0;
};

class segas18_astormbl_s18snd_state : public segas18_astormbl_state
{
public:
	segas18_astormbl_s18snd_state(const machine_config &mconfig, device_type type, const char *tag)
		: segas18_astormbl_state(mconfig, type, tag)
		, m_soundcpu(*this, "soundcpu")
		, m_soundbank(*this, "soundbank")
		, m_soundlatch(*this, "soundlatch")
	{ }

	void astormbl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask) override;

private:
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
	void pcm_map(address_map &map) ATTR_COLD;
	void soundbank_w(uint8_t data);
	void astormbl_sound(machine_config &config);

	required_device<cpu_device> m_soundcpu;
	required_memory_bank m_soundbank;
	required_device<generic_latch_8_device> m_soundlatch;
};

class segas18_astormbl_bootsnd_state : public segas18_astormbl_state
{
public:
	segas18_astormbl_bootsnd_state(const machine_config &mconfig, device_type type, const char *tag)
		: segas18_astormbl_state(mconfig, type, tag)
		, m_soundcpu(*this, "soundcpu")
		, m_soundlatch(*this, "soundlatch")
		, m_okibank(*this, "okibank")
	{ }

	void astormb2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask) override;

private:
	void sys18bl_okibank_w(uint8_t data);
	void sys18bl_sound_map(address_map &map) ATTR_COLD;
	void sys18bl_oki_map(address_map &map) ATTR_COLD;

	void astormb2_sound(machine_config &config);

	required_device<cpu_device> m_soundcpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_memory_bank m_okibank;
};


static INPUT_PORTS_START( astormbl )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )

/*  2nd Coin Slot listed as "Not Used" in Test Mode for Alien Storm Bootlegs  */

	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x00, "Special" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Chutes" )       PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END



TILE_GET_INFO_MEMBER(segas18_astormbl_state::get_text_tile_info)
{
	int tile_number = m_textram[tile_index];

	tileinfo.set(0,
			(tile_number & 0x1ff),
			(tile_number >> 9) & 0x7,
			0);

	tileinfo.category = (tile_number >> 15) & 1;
}

void segas18_astormbl_state::sys16_textram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);
	m_text_layer->mark_tile_dirty(offset);
}

void segas18_astormbl_state::video_start()
{
	m_text_layer = &machine().tilemap().create(
		*m_gfxdecode,
		tilemap_get_info_delegate(*this, FUNC(segas18_astormbl_state::get_text_tile_info)),
		TILEMAP_SCAN_ROWS,
		8, 8,
		64, 28);

	m_text_layer->set_transparent_pen(0);
	m_text_layer->set_scrollx(512 - 320);
}


void segas18_astormbl_state::draw_tile(screen_device& screen, bitmap_ind16& bitmap, const rectangle &cliprect, int tilenum, int colour, int xpos, int ypos, uint8_t pri, int transpen, bool opaque)
{
	gfx_element* gfx = m_gfxdecode->gfx(0);
	bitmap_ind8 &priority_bitmap = screen.priority();

	const uint8_t* gfxdat = gfx->get_data(tilenum);

	for (int y = 0; y < 8; y++)
	{
		int realypos = ypos + y;

		for (int x = 0; x < 8; x++)
		{
			int realxpos = xpos + x;

			if ((realypos >= cliprect.min_y) && (realypos <= cliprect.max_y) && (realxpos >= cliprect.min_x) && (realxpos <= cliprect.max_x))
			{
				uint16_t* dst = &bitmap.pix(realypos);
				uint8_t* pridst = &priority_bitmap.pix(realypos);
				uint8_t pix = gfxdat[(y*8) + x];

				if (pix || opaque)
				{
					dst[realxpos] = pix | (colour * 8);
					pridst[realxpos] = pri;
				}
			}
		}
	}
}

void segas18_astormbl_state::draw_layer(screen_device& screen, bitmap_ind16& bitmap, const rectangle &cliprect, int layer, bool opaque, int pri0, int pri1)
{
	for (int quadrant = 0; quadrant < 4; quadrant++)
	{
		int xbase = 0, ybase = 0;

		if (quadrant & 2)
			xbase = 0;
		else
			xbase = 512;

		xbase += 0x140;

		if (quadrant & 1)
			ybase = 256;
		else
			ybase = 0;

		if (layer == 0)
			ybase ^= 256;

		int yscroll = layer & 1 ? m_tileyscroll0[0] : m_tileyscroll1[0];

		for (int y = 0; y < 0x20; y++)
		{
			const uint16_t rowconf = m_tilestripconfig[(layer * 0x80) + (quadrant * 0x20) + y];

			uint8_t pagesource = (rowconf & 0xf000) >> 12;
			uint8_t rowtilebank = (rowconf & 0x0e00) >> 9;
			uint16_t rowscroll = (rowconf & 0x01ff);
			int yposn = (y * 8) + ybase - yscroll;

			yposn &= 0x1ff;
			if (yposn & 0x100) yposn -= 0x200;

			for (int x = 0; x < 0x40; x++)
			{
				uint16_t tiledat = m_tileram[(pagesource * 0x800) + x + (y * 0x40)];

				int tilenum = tiledat & 0x0fff;
				if (tiledat & 0x1000) tilenum |= (rowtilebank * 0x1000);

				int xposn = ((x * 8) - rowscroll) + xbase + 8;
				xposn &= 0x3ff;
				if (xposn & 0x200) xposn -= 0x400;

				int pri;
				if (tiledat & 0x8000)
					pri = pri1;
				else
					pri = pri0;

				draw_tile(screen, bitmap, cliprect, tilenum, (tiledat & 0x1fc0) >> 6, xposn, yposn, pri, 0, opaque);
			}
		}

	}
}

uint32_t segas18_astormbl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	// if no drawing is happening, fill with black and get out
	bitmap.fill(m_palette->black_pen(), cliprect);

	// whole video enable, or just tiles?
	if (!m_tileenable[0])
		return 0;

	// start the sprites drawing
	m_sprites->draw_async(cliprect);

	// reset priorities
	screen.priority().fill(0, cliprect);

	draw_layer(screen, bitmap, cliprect, 1, true, 0x01, 0x02);
	draw_layer(screen, bitmap, cliprect, 0, false, 0x02, 0x04);

	m_text_layer->draw(screen, bitmap, cliprect, 0, 0x04);
	m_text_layer->draw(screen, bitmap, cliprect, 1, 0x08);

	// mix in sprites
	bitmap_ind16 &sprites = m_sprites->bitmap();
	for (const sparse_dirty_rect *rect = m_sprites->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
	{
		for (int y = rect->min_y; y <= rect->max_y; y++)
		{
			uint16_t *dest = &bitmap.pix(y);
			uint16_t *src = &sprites.pix(y);
			uint8_t *pri = &screen.priority().pix(y);
			for (int x = rect->min_x; x <= rect->max_x; x++)
			{
				// only process written pixels
				uint16_t pix = src[x];
				if (pix != 0xffff)
				{
					// compare sprite priority against tilemap priority
					int priority = (pix >> 10) & 3;
					if ((1 << priority) > pri[x])
					{
						// if the color is set to maximum, shadow pixels underneath us
						if ((pix & 0x03f0) == 0x03f0)
							dest[x] += m_palette_entries;

						// otherwise, just add in sprite palette base
						else
							dest[x] = 0x400 | (pix & 0x3ff);
					}
				}
			}
		}
	}

	return 0;
}

static GFXDECODE_START( gfx_segas16b )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar,   0, 1024 )
GFXDECODE_END

void segas18_astormbl_state::astormbl_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x10ffff).ram().share("tileram");
	map(0x110000, 0x110fff).ram().w(FUNC(segas18_astormbl_state::sys16_textram_w)).share("textram");

	map(0x140000, 0x140fff).ram().w(FUNC(segas18_astormbl_state::paletteram_w)).share("paletteram");

	map(0x200000, 0x200fff).ram().share("sprites");

	map(0xa00000, 0xa00001).portr("COINAGE");
	map(0xa00002, 0xa00003).portr("DSW1");

	map(0xa00006, 0xa00007).w(FUNC(segas18_astormbl_state::sound_w));
	map(0xa0000e, 0xa0000f).nopw();// was tilebank, unused here? as tilebank is copied in the code per-row to the tilestripconfig area

	map(0xa01000, 0xa01001).portr("SERVICE");
	map(0xa01002, 0xa01003).portr("P1");
	map(0xa01004, 0xa01005).portr("P2");
	map(0xa01006, 0xa01007).portr("P3");

	map(0xa02100, 0xa02101).noprw();
	map(0xa03000, 0xa03001).noprw();

	map(0xc00000, 0xc00003).noprw(); // leftover reads/writes from the Genesis VDP - bootlegs don't have this
	map(0xc00004, 0xc00007).ram();

	map(0xc44000, 0xc44001).ram(); // probably a new video register

	map(0xc46000, 0xc46001).ram().share("tileyscroll1"); // y scroll?
	map(0xc46200, 0xc46201).ram().share("tileyscroll0");
	map(0xc46400, 0xc465ff).ram().share("tilestripconfig"); // per row page select, xscroll, tilebank
	map(0xc46600, 0xc46601).ram().share("tileenable");

	map(0xfe0020, 0xfe003f).nopw(); // leftover writes from memory mapped config registers - bootlegs don't have this

	map(0xffc000, 0xffffff).ram();
}

void segas18_astormbl_state::astormbl_video(machine_config &config)
{
	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_segas16b);
	PALETTE(config, m_palette).set_entries(2048*2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(25'174'800)/4, 400, 0, 320, 262, 0, 224);
	m_screen->set_screen_update(FUNC(segas18_astormbl_state::screen_update));
	m_screen->set_palette(m_palette);
	// see note in segas16a.cpp, also used here for consistency
	m_screen->set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);

	SEGA_SYS16B_SPRITES(config, m_sprites, 0);
	m_sprites->set_local_originx(64);
}

void segas18_astormbl_s18snd_state::sound_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x9fff).rom().region("soundcpu", 0);
	map(0xa000, 0xbfff).bankr("soundbank");
	map(0xc000, 0xdfff).m("rfsnd", FUNC(rf5c68_device::map));
	map(0xe000, 0xffff).ram();
}

void segas18_astormbl_s18snd_state::sound_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).mirror(0x0c).rw("ym1", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x90, 0x93).mirror(0x0c).rw("ym2", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0xa0, 0xa0).mirror(0x1f).w(FUNC(segas18_astormbl_s18snd_state::soundbank_w));
	//map(0xc0, 0xc0).mirror(0x1f).rw(m_mapper, FUNC(sega_315_5195_mapper_device::pread), FUNC(sega_315_5195_mapper_device::pwrite)); // no memory mapper on bootleg
	map(0xc0, 0xc0).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void segas18_astormbl_s18snd_state::pcm_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram();
}

void segas18_astormbl_s18snd_state::machine_start()
{
	segas18_astormbl_state::machine_start();

	m_soundbank->configure_entries(0, 256, memregion("soundcpu")->base(), 0x2000);
}

void segas18_astormbl_s18snd_state::soundbank_w(uint8_t data)
{
	m_soundbank->set_entry(data);
}

void segas18_astormbl_s18snd_state::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch->write(data & 0xff);
		m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

void segas18_astormbl_s18snd_state::astormbl_sound(machine_config &config)
{
	Z80(config, m_soundcpu, 8'000'000);
	m_soundcpu->set_addrmap(AS_PROGRAM, &segas18_astormbl_s18snd_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &segas18_astormbl_s18snd_state::sound_portmap);

	GENERIC_LATCH_8(config, m_soundlatch);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3438_device &ym1(YM3438(config, "ym1", 8'000'000));
	ym1.add_route(ALL_OUTPUTS, "mono", 0.40);
	ym1.irq_handler().set_inputline("soundcpu", INPUT_LINE_IRQ0);

	ym3438_device &ym2(YM3438(config, "ym2", 8'000'000));
	ym2.add_route(ALL_OUTPUTS, "mono", 0.40);

	rf5c68_device &rfsnd(RF5C68(config, "rfsnd", 10'000'000)); // ASSP (RF)5C68A
	rfsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
	rfsnd.set_addrmap(0, &segas18_astormbl_s18snd_state::pcm_map);
}

void segas18_astormbl_s18snd_state::astormbl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &segas18_astormbl_s18snd_state::astormbl_map);
	m_maincpu->set_vblank_int("screen", FUNC(segas18_astormbl_s18snd_state::irq4_line_hold));

	astormbl_video(config);
	astormbl_sound(config);
}

// --------------------

void segas18_astormbl_bootsnd_state::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_soundlatch->write(data & 0xff);
}

void segas18_astormbl_bootsnd_state::sys18bl_okibank_w(uint8_t data) // TODO: verify correctness
{
	m_okibank->set_entry(data & 0x07);
}

void segas18_astormbl_bootsnd_state::machine_start()
{
	segas18_astormbl_state::machine_start();

	m_okibank->configure_entries(0, 8, memregion("oki")->base() + 0x30000, 0x10000);
}

void segas18_astormbl_bootsnd_state::sys18bl_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x9000, 0x9000).w(FUNC(segas18_astormbl_bootsnd_state::sys18bl_okibank_w));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0x87ff).ram();
}

void segas18_astormbl_bootsnd_state::sys18bl_oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank");
}

void segas18_astormbl_bootsnd_state::astormb2_sound(machine_config &config)
{
	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, 0);

	Z80(config, m_soundcpu, 8_MHz_XTAL/2); // 4MHz
	m_soundcpu->set_addrmap(AS_PROGRAM, &segas18_astormbl_bootsnd_state::sys18bl_sound_map);

	// 1 OKI M6295 instead of original sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(8'000'000)/8, okim6295_device::PIN7_HIGH)); // 1MHz clock and pin verified
	oki.set_addrmap(0, &segas18_astormbl_bootsnd_state::sys18bl_oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}


void segas18_astormbl_bootsnd_state::astormb2(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL/2);  // 12MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &segas18_astormbl_bootsnd_state::astormbl_map);
	m_maincpu->set_vblank_int("screen", FUNC(segas18_astormbl_bootsnd_state::irq4_line_hold));

	astormbl_video(config);
	astormb2_sound(config);
}



ROM_START( astormbl )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "astorm.a6", 0x000000, 0x40000, CRC(7682ed3e) SHA1(b857352ad9c66488e91f60989472638c483e4ae8) )
	ROM_LOAD16_BYTE( "astorm.a5", 0x000001, 0x40000, CRC(efe9711e) SHA1(496fd9e30941fde1658fab7292a669ef7964cecb) )

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "epr13073.bin", 0x00000, 0x40000, CRC(df5d0a61) SHA1(79ad71de348f280bad847566c507b7a31f022292) )
	ROM_LOAD( "epr13074.bin", 0x40000, 0x40000, CRC(787afab8) SHA1(a119042bb2dad54e9733bfba4eaab0ac5fc0f9e7) )
	ROM_LOAD( "epr13075.bin", 0x80000, 0x40000, CRC(4e01b477) SHA1(4178ce4a87ea427c3b0195e64acef6cddfb3485f) )

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "mpr13082.bin", 0x000001, 0x40000, CRC(a782b704) SHA1(ba15bdfbc267b8d86f03e5310ce60846ff846de3) )
	ROM_LOAD16_BYTE( "astorm.a11",   0x000000, 0x40000, CRC(7829c4f3) SHA1(3adb7aa7f70163d3848c98316e18b9783c41d663) )
	ROM_LOAD16_BYTE( "mpr13081.bin", 0x080001, 0x40000, CRC(eb510228) SHA1(4cd387b160ec7050e1300ebe708853742169e643) )
	ROM_LOAD16_BYTE( "mpr13088.bin", 0x080000, 0x40000, CRC(3b6b4c55) SHA1(970495c54b3e1893ee8060f6ca1338c2cbbd1074) )
	ROM_LOAD16_BYTE( "mpr13080.bin", 0x100001, 0x40000, CRC(e668eefb) SHA1(d4a087a238b4d3ac2d23fe148d6a73018e348a89) )
	ROM_LOAD16_BYTE( "mpr13087.bin", 0x100000, 0x40000, CRC(2293427d) SHA1(4fd07763ff060afd594e3f64fa4750577f56c80e) )
	ROM_LOAD16_BYTE( "epr13079.bin", 0x180001, 0x40000, CRC(de9221ed) SHA1(5e2e434d1aa547be1e5652fc906d2e18c5122023) )
	ROM_LOAD16_BYTE( "epr13086.bin", 0x180000, 0x40000, CRC(8c9a71c4) SHA1(40b774765ac888792aad46b6351a24b7ef40d2dc) )

	// is this REALLY meant to be using the original system18 sound system?
	ROM_REGION( 0x200000, "soundcpu", ROMREGION_ERASEFF ) // sound CPU
	ROM_LOAD( "epr-13083.bin", 0x000000, 0x20000, CRC(5df3af20) SHA1(e49105fcfd5bf37d14bd760f6adca5ce2412883d) )
	ROM_LOAD( "epr-13076.bin", 0x080000, 0x40000, CRC(94e6c76e) SHA1(f99e58a9bf372c41af211bd9b9ea3ac5b924c6ed) )
	ROM_LOAD( "epr-13077.bin", 0x100000, 0x40000, CRC(e2ec0d8d) SHA1(225b0d223b7282cba7710300a877fb4a2c6dbabb) )
	ROM_LOAD( "epr-13078.bin", 0x180000, 0x40000, CRC(15684dc5) SHA1(595051006de24f791dae937584e502ff2fa31d9c) )
ROM_END

/*

CPUs:
on main board:

1x MC68000P10 (main)
1x Z8400BB1-Z80BCPU (sound)
1x OKI M6295 (sound)
1x oscillator 24.000MHz (close to main)
1x oscillator 8.000MHz (close to sound)

ROMs
on main board:

9x M27C512 (1,2,3,4,5,6,7,8,10)
1x TMS27C256 (9)
21x NM27C010Q (11-31)

----------------------

on roms board:
6x NM27C010Q (32-37)
2x N82S123N

*/

ROM_START( astormb2 )
	ROM_REGION( 0x080000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "1.a4", 0x000000, 0x10000, CRC(cca0d0af) SHA1(26fdbbeb8444d05f0ca2056a7c7fb81b0f1f2b5a) )
	ROM_LOAD16_BYTE( "2.a3", 0x020000, 0x10000, CRC(f95eb883) SHA1(b25d9c0fd46a534e7612f4a3ffa708b73654ae2b) )
	ROM_LOAD16_BYTE( "3.a2", 0x040000, 0x10000, CRC(4206ecd4) SHA1(45c65d7727cfaf215a7081159f6931185e92b39a) ) // epr13182.bin [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "4.a1", 0x060000, 0x10000, CRC(23247c95) SHA1(e4d78c453d2cb77946dd1b5266de823968eade77) ) // epr13182.bin [4/4]      98.648071%
	ROM_LOAD16_BYTE( "5.a9", 0x000001, 0x10000, CRC(6143039e) SHA1(8a5143c1e2c637149e988c423fa30b31e29a1193) )
	ROM_LOAD16_BYTE( "6.a8", 0x020001, 0x10000, CRC(0fd17bec) SHA1(e9a5dd93394fdf1561a925e4111dfce51b717b14) )
	ROM_LOAD16_BYTE( "7.a7", 0x040001, 0x10000, CRC(c901e228) SHA1(f459ba819a4e5f5174ff1b3957fb648c93beed53) ) // epr13181.bin [3/4]      IDENTICAL
	ROM_LOAD16_BYTE( "8.a6", 0x060001, 0x10000, CRC(bfb9d607) SHA1(8c3e10c1397fa0807d8df4715c9eb1945c774924) ) // epr13181.bin [4/4]      98.587036%

	ROM_REGION( 0xc0000, "gfx1", 0 ) // tiles
	ROM_LOAD( "32.01",  0x00000, 0x20000, CRC(d2aeb4ab) SHA1(9338ec5dc48f5d2b20511628a281236fe4646ef4) ) // epr13073.bin [1/2]      IDENTICAL
	ROM_LOAD( "33.011", 0x20000, 0x20000, CRC(2193f0ae) SHA1(84070f74693699c1ffc1a47517a97b5d058d08ec) ) // epr13073.bin [2/2]      IDENTICAL
	ROM_LOAD( "34.02",  0x40000, 0x20000, CRC(849aa725) SHA1(0f949dfe8a6c5796edc86a05339da80a158a95ae) ) // epr13074.bin [1/2]      IDENTICAL
	ROM_LOAD( "35.021", 0x60000, 0x20000, CRC(3f190347) SHA1(131953ccefb95eeef1ea90499ce521c3749f95c1) ) // epr13074.bin [2/2]      IDENTICAL
	ROM_LOAD( "36.03",  0x80000, 0x20000, CRC(c0f9628d) SHA1(aeacf5e409adfa0b9c28c90d4e89eb1f56cd5f4d) ) // epr13075.bin [1/2]      IDENTICAL
	ROM_LOAD( "37.031", 0xa0000, 0x20000, CRC(95af904e) SHA1(6574fa874c355c368109b417aab7d0b05c9d215d) ) // epr13075.bin [2/2]      IDENTICAL

	ROM_REGION16_BE( 0x200000, "sprites", 0 ) // sprites
	ROM_LOAD16_BYTE( "17.042", 0x000001, 0x20000, CRC(db08beb5) SHA1(c154d22c69b77637d6a9d0f2bffcfb47e6901ec8) ) // mpr13082.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "16.043", 0x040001, 0x20000, CRC(41f78977) SHA1(9cf9fcf96722d148c4b2cf7aa33425b6efcd0379) ) // mpr13082.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "29.012", 0x000000, 0x20000, CRC(22acf675) SHA1(80fd0d96017bf36d964a79f7e13e73fee7ed370a) ) // mpr13089.bin [1/2]      99.941254%
	ROM_LOAD16_BYTE( "28.013", 0x040000, 0x20000, CRC(32b37a3a) SHA1(70f268aa99a17739fd9d832b5f1d9e37247747e6) ) // mpr13089.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "19.040", 0x080001, 0x20000, CRC(10c359ac) SHA1(9087cb824242ce5fc8eba45b61cca8b329c576e5) ) // mpr13081.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "18.041", 0x0c0001, 0x20000, CRC(47146c1d) SHA1(cd5d92136f86128a9f304c4f8850f1efd652dd5c) ) // mpr13081.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "31.010", 0x080000, 0x20000, CRC(e88fc39c) SHA1(f19c55c49771625a76e65b639a3b23969db8031d) ) // mpr13088.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "30.011", 0x0c0000, 0x20000, CRC(6fe7e2a2) SHA1(94e5852377f72fd00daae302db4a5f93301213e4) ) // mpr13088.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "21.032", 0x100001, 0x20000, CRC(c9e5a258) SHA1(809a3a3f88efe9c7a9dd9f6439ccb48fffb84df0) ) // mpr13080.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "20.033", 0x140001, 0x20000, CRC(ddf8d00e) SHA1(0a9031063921bb03e7fd57eea369a1ddcfa85431) ) // mpr13080.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "25.022", 0x100000, 0x20000, CRC(af8f3700) SHA1(3787f732eee5c6c9b6550bd4ce5387aff2c4072e) ) // mpr13087.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "24.023", 0x140000, 0x20000, CRC(a092ecb6) SHA1(d7cc85eaea70c7947c497bc1d9743ab12a6fb43e) ) // mpr13087.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "23.030", 0x180001, 0x20000, CRC(adc1b625) SHA1(496a1e92a833dbde37a0426165ff4250848b6ef4) ) // epr13079.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "22.031", 0x1c0001, 0x20000, CRC(27c27f38) SHA1(439502250da4e376d2aa4bd9122455c6991e334d) ) // epr13079.bin [2/2]      IDENTICAL
	ROM_LOAD16_BYTE( "27.020", 0x180000, 0x20000, CRC(6c5312aa) SHA1(94b74c78f318fcc1881a2926cebc98033a7e535d) ) // epr13086.bin [1/2]      IDENTICAL
	ROM_LOAD16_BYTE( "26.021", 0x1c0000, 0x20000, CRC(c67fc986) SHA1(5fac826f9dde45201e3b93582dbe29c584a10229) ) // epr13086.bin [2/2]      99.987030%

	// Sound HW is very different to the originals
	ROM_REGION( 0x08000, "soundcpu", 0 ) // Z80 sound CPU
	ROM_LOAD( "9.a5", 0x00000, 0x08000, CRC(0a4638e9) SHA1(0470e03a194464ff53c7583637193b585f5fd79f) )

	ROM_REGION( 0xb0000, "oki", 0 ) // Oki6295 Samples
	ROM_LOAD( "11.a10", 0x00000, 0x20000, CRC(7e0f752c) SHA1(a4070c3fa4848b5be223f9b927de4b6926dbb4e6) ) // contains sample table
	ROM_LOAD( "10.a11", 0x20000, 0x10000, CRC(722e5969) SHA1(9cf891c6533b2e2a5c4741aa4e405038a7bf4e97) ) // sound effects
	// BGM (banked 0x30000-0x3ffff)
	ROM_LOAD( "15.a12", 0x30000, 0x20000, CRC(dffde929) SHA1(037b32470747d155385e532ee574b1234b3c2b26) )
	ROM_LOAD( "14.a13", 0x50000, 0x20000, CRC(07e6b3a5) SHA1(32da2a9aeb840b76e6f0117ac342ff5d612762b4) )
	ROM_LOAD( "13.a14", 0x70000, 0x20000, CRC(c60d6f18) SHA1(c9610729f19ae8414efd785948a1e6fb079bfe8d) )
	ROM_LOAD( "12.a15", 0x90000, 0x20000, CRC(cb4517db) SHA1(4c93376c2b3e70001bbc283d4485bb55514f6ef9) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "n82s129n.129",  0x0000, 0x0100, CRC(a7c22d96) SHA1(160deae8053b09c09328325246598b3518c7e20b) )
	ROM_LOAD( "n82s123n.123",  0x0100, 0x0020, CRC(58bcf8bd) SHA1(e4d3d179b08c0f3424a6bec0f15058fb1b56f8d8) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD("pal1.bin", 0x000, 0x117, CRC(9b362047) SHA1(1e6db7d6b8a6e4c321e3245f36f5b794f22e880e) ) // GAL16V8S-20HB1 on main PCB
	ROM_LOAD("pal2.bin", 0x200, 0x117, CRC(b7eecbd4) SHA1(2ba89ec54a628abe08750ff6ba6eff5845fe0d30) ) // GAL16V8S-20HB1 on video PCB
	ROM_LOAD("pal3.bin", 0x400, 0x117, NO_DUMP ) // GAL16V8S-20HB1 on video PCB
ROM_END

} // anonymous namespace

GAME( 1990, astormbl,    astorm,    astormbl,      astormbl, segas18_astormbl_s18snd_state,   empty_init,   ROT0,   "bootleg", "Alien Storm (bootleg, set 1)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, astormb2,    astorm,    astormb2,      astormbl, segas18_astormbl_bootsnd_state,  empty_init,   ROT0,   "bootleg", "Alien Storm (bootleg, set 2)", MACHINE_IMPERFECT_GRAPHICS ) // sound verified on real hardware
