// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood,Angelo Salese
/***************************************************************************

    Imagetek I4100 / I4220 / I4300 device files

    device emulation by Angelo Salese, based off from original metro.cpp
    implementation by Luca Elia & David Haywood

    TODO:
    - interrupt enable/acknowledge/vector;
    - soundlatch delegate;
    - inputs for i4300;
    - hyprduel.cpp uses scanline attribute which crawls to unusable state
      with current video routines here;
    - CRT Controller, also understand why it needs so many writes before actual parameters;
    - Wrong color bars in service mode (e.g. balcube, toride2g). They use solid color tiles (80xx),
      but the right palette is not at 00-ff.
      Related to the unknown table in the RAM mapped just before the palette?
      Update: the colors should have a common bank of 0xb (so 0x8bxx), it's unknown why the values
      diverges, the blitter is responsible of that upload fwiw;
    - Some gfx problems in ladykill, 3kokushi, puzzli, gakusai, seem related to how we handle
      windows, wrapping, read-modify-write areas;
    - puzzli: emulate hblank irq and fix video routines here (water effect not emulated?);

============================================================================

                    driver by   Luca Elia (l.elia@tin.it)

                            [ 3 Scrolling Layers ]

        There is memory for a huge layer, but the actual tilemap
        is a smaller window (of fixed size) carved from anywhere
        inside that layer.

        Tile Size:                  8 x 8 x 4
        (later games can switch to  8 x 8 x 8, 16 x 16 x 4/8 at run time)

        Big Layer Size:         2048 x 2048 (8x8 tiles) or 4096 x 4096 (16x16 tiles)

        Tilemap Window Size:    512 x 256 (8x8 tiles) or 1024 x 512 (16x16 tiles)

        The tile codes in memory do not map directly to tiles. They
        are indexes into a table (with 0x200 entries) that defines
        a virtual set of tiles for the 3 layers. Each entry in that
        table adds 16 tiles to the set of available tiles, and decides
        their color code.

        Tile code with their msbit set are different as they mean:
        draw a tile filled with a single color (0-fff)


                            [ 512 Zooming Sprites ]

        The sprites are NOT tile based: the "tile" size can vary from
        8 to 64 (independently for width and height) with an 8 pixel
        granularity. The "tile" address is a multiple of 8x8 pixels.

        Each sprite can be shrinked to ~1/4 or enlarged to ~32x following
        an exponential curve of sizes (with one zoom value for both width
        and height)


***************************************************************************/

#include "emu.h"
#include "imagetek_i4100.h"

#include <algorithm>

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

/***************************************************************************
                            Graphics Layouts
***************************************************************************/

/* 8x8x4 tiles */
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6 },
	{ STEP8(0,4*8) },
	4*8*8
};

/* 8x8x8 tiles for later games */
static GFXLAYOUT_RAW( layout_8x8x8, 8, 8, 8*8, 32*8 )

/* 16x16x4 tiles for later games */
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6, 4*9,4*8, 4*11,4*10, 4*13,4*12, 4*15,4*14 },
	{ STEP16(0,4*16) },
	4*8*8
};

/* 16x16x8 tiles for later games */
static GFXLAYOUT_RAW( layout_16x16x8, 16, 16, 16*8, 32*8 )

GFXDECODE_START( imagetek_i4100_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
GFXDECODE_END

GFXDECODE_START( imagetek_i4100_device::gfxinfo_ext )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout_8x8x4,    0x0, 0x100 ) // [0] 4 Bit Tiles
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout_8x8x8,    0x0,  0x10 ) // [1] 8 Bit Tiles
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout_16x16x4,  0x0, 0x100 ) // [2] 4 Bit Tiles 16x16
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout_16x16x8,  0x0,  0x10 ) // [3] 8 Bit Tiles 16x16
GFXDECODE_END

// device type definition
DEFINE_DEVICE_TYPE(I4100, imagetek_i4100_device, "i4100", "Imagetek I4100 052 VDP")
DEFINE_DEVICE_TYPE(I4220, imagetek_i4220_device, "i4220", "Imagetek I4220 071 VDP")
DEFINE_DEVICE_TYPE(I4300, imagetek_i4300_device, "i4300", "Imagetek I4300 095 VDP")

void imagetek_i4100_device::map(address_map &map)
{
	map(0x00000, 0x1ffff).rw(FUNC(imagetek_i4100_device::vram_0_r), FUNC(imagetek_i4100_device::vram_0_w)).share("vram_0");
	map(0x20000, 0x3ffff).rw(FUNC(imagetek_i4100_device::vram_1_r), FUNC(imagetek_i4100_device::vram_1_w)).share("vram_1");
	map(0x40000, 0x5ffff).rw(FUNC(imagetek_i4100_device::vram_2_r), FUNC(imagetek_i4100_device::vram_2_w)).share("vram_2");
	map(0x60000, 0x6ffff).r(FUNC(imagetek_i4100_device::gfxrom_r));
	map(0x70000, 0x71fff).rw(FUNC(imagetek_i4100_device::scratchram_r), FUNC(imagetek_i4100_device::scratchram_w)).share("scratchram"); // unknown, maybe palette
	map(0x72000, 0x73fff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette");
	map(0x74000, 0x74fff).rw(FUNC(imagetek_i4100_device::spriteram_r), FUNC(imagetek_i4100_device::spriteram_w)).share("spriteram");
	map(0x75000, 0x75fff).rw(FUNC(imagetek_i4100_device::rmw_vram_0_r), FUNC(imagetek_i4100_device::rmw_vram_0_w));
	map(0x76000, 0x76fff).rw(FUNC(imagetek_i4100_device::rmw_vram_1_r), FUNC(imagetek_i4100_device::rmw_vram_1_w));
	map(0x77000, 0x77fff).rw(FUNC(imagetek_i4100_device::rmw_vram_2_r), FUNC(imagetek_i4100_device::rmw_vram_2_w));
	map(0x78000, 0x787ff).rw(FUNC(imagetek_i4100_device::tiletable_r), FUNC(imagetek_i4100_device::tiletable_w)).share("tiletable");
	// video registers
	map(0x78800, 0x78801).rw(FUNC(imagetek_i4100_device::sprite_count_r), FUNC(imagetek_i4100_device::sprite_count_w));
	map(0x78802, 0x78803).rw(FUNC(imagetek_i4100_device::sprite_priority_r), FUNC(imagetek_i4100_device::sprite_priority_w));
	map(0x78804, 0x78805).rw(FUNC(imagetek_i4100_device::sprite_yoffset_r), FUNC(imagetek_i4100_device::sprite_yoffset_w));
	map(0x78806, 0x78807).rw(FUNC(imagetek_i4100_device::sprite_xoffset_r), FUNC(imagetek_i4100_device::sprite_xoffset_w));
	map(0x78808, 0x78809).rw(FUNC(imagetek_i4100_device::sprite_color_code_r), FUNC(imagetek_i4100_device::sprite_color_code_w));
	map(0x78810, 0x78811).rw(FUNC(imagetek_i4100_device::layer_priority_r), FUNC(imagetek_i4100_device::layer_priority_w));
	map(0x78812, 0x78813).rw(FUNC(imagetek_i4100_device::background_color_r), FUNC(imagetek_i4100_device::background_color_w));

	map(0x78840, 0x7884d).w(FUNC(imagetek_i4100_device::blitter_w)).share("blitter_regs");
	map(0x78850, 0x78851).rw(FUNC(imagetek_i4100_device::screen_yoffset_r), FUNC(imagetek_i4100_device::screen_yoffset_w));
	map(0x78852, 0x78853).rw(FUNC(imagetek_i4100_device::screen_xoffset_r), FUNC(imagetek_i4100_device::screen_xoffset_w));
	map(0x78860, 0x7886b).rw(FUNC(imagetek_i4100_device::window_r), FUNC(imagetek_i4100_device::window_w)).share("windowregs");
	map(0x78870, 0x7887b).rw(FUNC(imagetek_i4100_device::scroll_r), FUNC(imagetek_i4100_device::scroll_w)).share("scrollregs");

	map(0x78880, 0x78881).w(FUNC(imagetek_i4100_device::crtc_vert_w));
	map(0x78890, 0x78891).w(FUNC(imagetek_i4100_device::crtc_horz_w));
	map(0x788a0, 0x788a1).w(FUNC(imagetek_i4100_device::crtc_unlock_w));
	map(0x788aa, 0x788ab).w(FUNC(imagetek_i4100_device::rombank_w));
	map(0x788ac, 0x788ad).w(FUNC(imagetek_i4100_device::screen_ctrl_w));
}

// same as above but with moved video registers (now at 0x797**)
void imagetek_i4220_device::v2_map(address_map &map)
{
	map(0x00000, 0x1ffff).rw(FUNC(imagetek_i4220_device::vram_0_r), FUNC(imagetek_i4220_device::vram_0_w)).share("vram_0");
	map(0x20000, 0x3ffff).rw(FUNC(imagetek_i4220_device::vram_1_r), FUNC(imagetek_i4220_device::vram_1_w)).share("vram_1");
	map(0x40000, 0x5ffff).rw(FUNC(imagetek_i4220_device::vram_2_r), FUNC(imagetek_i4220_device::vram_2_w)).share("vram_2");
	map(0x60000, 0x6ffff).r(FUNC(imagetek_i4220_device::gfxrom_r));
	map(0x70000, 0x71fff).rw(FUNC(imagetek_i4220_device::scratchram_r), FUNC(imagetek_i4220_device::scratchram_w)).share("scratchram"); // unknown, maybe palette
	map(0x72000, 0x73fff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette");
	map(0x74000, 0x74fff).rw(FUNC(imagetek_i4220_device::spriteram_r), FUNC(imagetek_i4220_device::spriteram_w)).share("spriteram");
	map(0x75000, 0x75fff).rw(FUNC(imagetek_i4220_device::rmw_vram_0_r), FUNC(imagetek_i4220_device::rmw_vram_0_w));
	map(0x76000, 0x76fff).rw(FUNC(imagetek_i4220_device::rmw_vram_1_r), FUNC(imagetek_i4220_device::rmw_vram_1_w));
	map(0x77000, 0x77fff).rw(FUNC(imagetek_i4220_device::rmw_vram_2_r), FUNC(imagetek_i4220_device::rmw_vram_2_w));
	map(0x78000, 0x787ff).rw(FUNC(imagetek_i4220_device::tiletable_r), FUNC(imagetek_i4220_device::tiletable_w)).share("tiletable");

	map(0x78840, 0x7884d).w(FUNC(imagetek_i4220_device::blitter_w)).share("blitter_regs");
	map(0x78850, 0x78851).rw(FUNC(imagetek_i4220_device::screen_yoffset_r), FUNC(imagetek_i4220_device::screen_yoffset_w));
	map(0x78852, 0x78853).rw(FUNC(imagetek_i4220_device::screen_xoffset_r), FUNC(imagetek_i4220_device::screen_xoffset_w));
	map(0x78860, 0x7886b).rw(FUNC(imagetek_i4220_device::window_r), FUNC(imagetek_i4220_device::window_w)).share("windowregs");
	map(0x78870, 0x7887b).rw(FUNC(imagetek_i4220_device::scroll_r), FUNC(imagetek_i4220_device::scroll_w)).share("scrollregs");

	map(0x78880, 0x78881).w(FUNC(imagetek_i4220_device::crtc_vert_w));
	map(0x78890, 0x78891).w(FUNC(imagetek_i4220_device::crtc_horz_w));
	map(0x788a0, 0x788a1).w(FUNC(imagetek_i4220_device::crtc_unlock_w));
	map(0x788aa, 0x788ab).w(FUNC(imagetek_i4220_device::rombank_w));
	map(0x788ac, 0x788ad).w(FUNC(imagetek_i4220_device::screen_ctrl_w));

	// video registers
	map(0x79700, 0x79701).rw(FUNC(imagetek_i4220_device::sprite_count_r), FUNC(imagetek_i4220_device::sprite_count_w));
	map(0x79702, 0x79703).rw(FUNC(imagetek_i4220_device::sprite_priority_r), FUNC(imagetek_i4220_device::sprite_priority_w));
	map(0x79704, 0x79705).rw(FUNC(imagetek_i4220_device::sprite_yoffset_r), FUNC(imagetek_i4220_device::sprite_yoffset_w));
	map(0x79706, 0x79707).rw(FUNC(imagetek_i4220_device::sprite_xoffset_r), FUNC(imagetek_i4220_device::sprite_xoffset_w));
	map(0x79708, 0x79709).rw(FUNC(imagetek_i4220_device::sprite_color_code_r), FUNC(imagetek_i4220_device::sprite_color_code_w));
	map(0x79710, 0x79711).rw(FUNC(imagetek_i4220_device::layer_priority_r), FUNC(imagetek_i4220_device::layer_priority_w));
	map(0x79712, 0x79713).rw(FUNC(imagetek_i4220_device::background_color_r), FUNC(imagetek_i4220_device::background_color_w));
	// repeated here in Puzzlet compatibility mode
	map(0x78800, 0x78801).rw(FUNC(imagetek_i4220_device::sprite_count_r), FUNC(imagetek_i4220_device::sprite_count_w));
	// ... this one breaks Blazing Tornado tho
//  AM_RANGE(0x78802, 0x78803) AM_READWRITE(sprite_priority_r,   sprite_priority_w)
	map(0x78804, 0x78805).rw(FUNC(imagetek_i4220_device::sprite_yoffset_r), FUNC(imagetek_i4220_device::sprite_yoffset_w));
	map(0x78806, 0x78807).rw(FUNC(imagetek_i4220_device::sprite_xoffset_r), FUNC(imagetek_i4220_device::sprite_xoffset_w));
	map(0x78808, 0x78809).rw(FUNC(imagetek_i4220_device::sprite_color_code_r), FUNC(imagetek_i4220_device::sprite_color_code_w));
	map(0x78810, 0x78811).rw(FUNC(imagetek_i4220_device::layer_priority_r), FUNC(imagetek_i4220_device::layer_priority_w));
	map(0x78812, 0x78813).rw(FUNC(imagetek_i4220_device::background_color_r), FUNC(imagetek_i4220_device::background_color_w));
}

// more changes around, namely the screen offsets being reversed here
void imagetek_i4300_device::v3_map(address_map &map)
{
	map(0x00000, 0x1ffff).rw(FUNC(imagetek_i4300_device::vram_0_r), FUNC(imagetek_i4300_device::vram_0_w)).share("vram_0");
	map(0x20000, 0x3ffff).rw(FUNC(imagetek_i4300_device::vram_1_r), FUNC(imagetek_i4300_device::vram_1_w)).share("vram_1");
	map(0x40000, 0x5ffff).rw(FUNC(imagetek_i4300_device::vram_2_r), FUNC(imagetek_i4300_device::vram_2_w)).share("vram_2");
	map(0x60000, 0x6ffff).r(FUNC(imagetek_i4300_device::gfxrom_r));
	map(0x70000, 0x71fff).rw(FUNC(imagetek_i4300_device::scratchram_r), FUNC(imagetek_i4300_device::scratchram_w)).share("scratchram"); // unknown, maybe palette
	map(0x72000, 0x73fff).rw(m_palette, FUNC(palette_device::read16), FUNC(palette_device::write16)).share("palette");
	map(0x74000, 0x74fff).rw(FUNC(imagetek_i4300_device::spriteram_r), FUNC(imagetek_i4300_device::spriteram_w)).share("spriteram");
	map(0x75000, 0x75fff).rw(FUNC(imagetek_i4300_device::rmw_vram_0_r), FUNC(imagetek_i4300_device::rmw_vram_0_w));
	map(0x76000, 0x76fff).rw(FUNC(imagetek_i4300_device::rmw_vram_1_r), FUNC(imagetek_i4300_device::rmw_vram_1_w));
	map(0x77000, 0x77fff).rw(FUNC(imagetek_i4300_device::rmw_vram_2_r), FUNC(imagetek_i4300_device::rmw_vram_2_w));
	map(0x78000, 0x787ff).rw(FUNC(imagetek_i4300_device::tiletable_r), FUNC(imagetek_i4300_device::tiletable_w)).share("tiletable");
	map(0x78808, 0x78809).rw(FUNC(imagetek_i4300_device::screen_xoffset_r), FUNC(imagetek_i4300_device::screen_xoffset_w));
	map(0x7880a, 0x7880b).rw(FUNC(imagetek_i4300_device::screen_yoffset_r), FUNC(imagetek_i4300_device::screen_yoffset_w));
	map(0x7880e, 0x7880f).w(FUNC(imagetek_i4300_device::screen_ctrl_w)); // TODO: can be read back here (gakusai)

	map(0x78800, 0x78801).w(FUNC(imagetek_i4300_device::crtc_unlock_w));
	map(0x78802, 0x78803).w(FUNC(imagetek_i4300_device::crtc_horz_w));
	map(0x78804, 0x78805).w(FUNC(imagetek_i4300_device::crtc_vert_w));

	map(0x78840, 0x7884d).w(FUNC(imagetek_i4300_device::blitter_w)).share("blitter_regs");
	map(0x78850, 0x7885b).rw(FUNC(imagetek_i4300_device::scroll_r), FUNC(imagetek_i4300_device::scroll_w)).share("scrollregs");
	map(0x78860, 0x7886b).rw(FUNC(imagetek_i4300_device::window_r), FUNC(imagetek_i4300_device::window_w)).share("windowregs");
	map(0x78870, 0x78871).w(FUNC(imagetek_i4300_device::rombank_w));

	// video registers
	map(0x79700, 0x79701).rw(FUNC(imagetek_i4300_device::sprite_count_r), FUNC(imagetek_i4300_device::sprite_count_w));
	map(0x79702, 0x79703).rw(FUNC(imagetek_i4300_device::sprite_priority_r), FUNC(imagetek_i4300_device::sprite_priority_w));
	map(0x79704, 0x79705).rw(FUNC(imagetek_i4300_device::sprite_yoffset_r), FUNC(imagetek_i4300_device::sprite_yoffset_w));
	map(0x79706, 0x79707).rw(FUNC(imagetek_i4300_device::sprite_xoffset_r), FUNC(imagetek_i4300_device::sprite_xoffset_w));
	map(0x79708, 0x79709).rw(FUNC(imagetek_i4300_device::sprite_color_code_r), FUNC(imagetek_i4300_device::sprite_color_code_w));
	map(0x79710, 0x79711).rw(FUNC(imagetek_i4300_device::layer_priority_r), FUNC(imagetek_i4300_device::layer_priority_w));
	map(0x79712, 0x79713).rw(FUNC(imagetek_i4300_device::background_color_r), FUNC(imagetek_i4300_device::background_color_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i4100_device - constructor
//-------------------------------------------------


imagetek_i4100_device::imagetek_i4100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool has_ext_tiles)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, has_ext_tiles ? gfxinfo_ext : gfxinfo, "palette")
	, device_video_interface(mconfig, *this)
	, m_vram_0(*this, "vram_0")
	, m_vram_1(*this, "vram_1")
	, m_vram_2(*this, "vram_2")
	, m_scratchram(*this, "scratchram")
	, m_blitter_regs(*this, "blitter_regs")
	, m_spriteram(*this, "spriteram")
	, m_tiletable(*this, "tiletable")
	, m_window(*this, "windowregs")
	, m_scroll(*this, "scrollregs")
	, m_palette(*this, "palette")
	, m_gfxrom(*this, DEVICE_SELF)
	, m_blit_irq_cb(*this)
	, m_support_8bpp( has_ext_tiles )
	, m_support_16x16( has_ext_tiles )
	, m_tilemap_scrolldx{0, 0, 0}
	, m_tilemap_scrolldy{0, 0, 0}
	, m_spriteram_buffered(false)
{
}

imagetek_i4100_device::imagetek_i4100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: imagetek_i4100_device(mconfig, I4100, tag, owner, clock, false)
{
}


imagetek_i4220_device::imagetek_i4220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: imagetek_i4100_device(mconfig, I4220, tag, owner, clock, true)
{
}

imagetek_i4300_device::imagetek_i4300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: imagetek_i4100_device(mconfig, I4300, tag, owner, clock, true)
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void imagetek_i4100_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_format(palette_device::GRBx_555, 0x1000);
	BUFFERED_SPRITERAM16(config, m_spriteram);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void imagetek_i4100_device::expand_gfx1()
{
	// TODO: remove from device_reset (otherwise you get broken sprites in i4220+ games because gfx rom isn't yet inited!)
	if(m_inited_hack == true)
		return;

	m_inited_hack = true;
	uint32_t length   =   m_gfxrom_size * 2;

	m_expanded_gfx1 = std::make_unique<uint8_t[]>(length);

	for (int i = 0; i < length; i += 2)
	{
		uint8_t src = m_gfxrom[i / 2];

		m_expanded_gfx1[i + 0] = src & 0xf;
		m_expanded_gfx1[i + 1] = src >> 4;
	}
}

void imagetek_i4100_device::device_start()
{
	m_inited_hack = false;
	save_item(NAME(m_rombank));
	save_item(NAME(m_crtc_unlock));
	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_priority));
	save_item(NAME(m_sprite_color_code));
	save_item(NAME(m_sprite_xoffset));
	save_item(NAME(m_sprite_yoffset));
	save_item(NAME(m_screen_xoffset));
	save_item(NAME(m_screen_yoffset));
	save_item(NAME(m_layer_priority));
	save_item(NAME(m_background_color));
	save_item(NAME(m_screen_blank));
	save_item(NAME(m_screen_flip));
	save_item(NAME(m_layer_tile_select));

//  memory_region *devregion =  machine().root_device().memregion(":gfx1");
//  m_gfxrom = devregion->base();
	if (m_gfxrom == nullptr)
		fatalerror("Imagetek i4100 %s: \"gfx1\" memory base not found",this->tag());

	m_gfxrom_size = m_gfxrom.bytes();

	m_blit_irq_cb.resolve_safe();
	m_blit_done_timer = timer_alloc(TIMER_BLIT_END);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void imagetek_i4100_device::device_reset()
{
	expand_gfx1();
}


void imagetek_i4100_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_BLIT_END:
			m_blit_irq_cb(ASSERT_LINE);
			break;
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER(imagetek_i4100_device::vram_0_r){ return m_vram_0[offset]; }
READ16_MEMBER(imagetek_i4100_device::vram_1_r){ return m_vram_1[offset]; }
READ16_MEMBER(imagetek_i4100_device::vram_2_r){ return m_vram_2[offset]; }
WRITE16_MEMBER(imagetek_i4100_device::vram_0_w){ COMBINE_DATA(&m_vram_0[offset]); }
WRITE16_MEMBER(imagetek_i4100_device::vram_1_w){ COMBINE_DATA(&m_vram_1[offset]); }
WRITE16_MEMBER(imagetek_i4100_device::vram_2_w){ COMBINE_DATA(&m_vram_2[offset]); }

/* Some game uses almost only the blitter to write to the tilemaps.
   The CPU can only access a "window" of 512x256 pixels in the upper
   left corner of the big tilemap */
// TODO: Puzzlet, Sankokushi & Lady Killer contradicts with aformentioned description (more like RMW?)

#define RMW_OFFS( _x_ ) ((_x_) & (0x3f)) + (((_x_) & ~(0x3f)) * (0x100 / 0x40))

READ16_MEMBER(imagetek_i4100_device::rmw_vram_0_r){ return m_vram_0[RMW_OFFS(offset)]; }
READ16_MEMBER(imagetek_i4100_device::rmw_vram_1_r){ return m_vram_1[RMW_OFFS(offset)]; }
READ16_MEMBER(imagetek_i4100_device::rmw_vram_2_r){ return m_vram_2[RMW_OFFS(offset)]; }
WRITE16_MEMBER(imagetek_i4100_device::rmw_vram_0_w){ COMBINE_DATA(&m_vram_0[RMW_OFFS(offset)]); }
WRITE16_MEMBER(imagetek_i4100_device::rmw_vram_1_w){ COMBINE_DATA(&m_vram_1[RMW_OFFS(offset)]); }
WRITE16_MEMBER(imagetek_i4100_device::rmw_vram_2_w){ COMBINE_DATA(&m_vram_2[RMW_OFFS(offset)]); }

#undef RMW_OFFS

READ16_MEMBER(imagetek_i4100_device::scratchram_r ) { return m_scratchram[offset]; }
WRITE16_MEMBER(imagetek_i4100_device::scratchram_w ) { COMBINE_DATA(&m_scratchram[offset]); }
READ16_MEMBER(imagetek_i4100_device::spriteram_r ) { return m_spriteram->live()[offset]; }
WRITE16_MEMBER(imagetek_i4100_device::spriteram_w ) { COMBINE_DATA(&m_spriteram->live()[offset]); }
READ16_MEMBER(imagetek_i4100_device::tiletable_r ) { return m_tiletable[offset]; }
WRITE16_MEMBER(imagetek_i4100_device::tiletable_w ) { COMBINE_DATA(&m_tiletable[offset]); }

// video registers
/*************************************************************
 *
 * 0.w  ---- ---- ---- ----     Number Of Sprites To Draw
 *
 ************************************************************/
READ16_MEMBER(imagetek_i4100_device::sprite_count_r) { return m_sprite_count; }
WRITE16_MEMBER(imagetek_i4100_device::sprite_count_w) { COMBINE_DATA(&m_sprite_count); }

/*************************************************************
 *
 * 2.w  f--- ---- ---- ----     Disable Sprites Layer Priority
 *      -edc ---- ---- ----
 *      ---- ba-- ---- ----     Sprites Masked Layer
 *      ---- --98 ---- ----     Sprites Priority
 *      ---- ---- 765- ----
 *      ---- ---- ---4 3210     Sprites Masked Number
 *
 *************************************************************/
READ16_MEMBER(imagetek_i4100_device::sprite_priority_r) { return m_sprite_priority; }
WRITE16_MEMBER(imagetek_i4100_device::sprite_priority_w) { COMBINE_DATA(&m_sprite_priority); }

/*************************************************************
 *
 * 4.w  ---- ---- ---- ----     Sprites Y Offset
 * 6.w  ---- ---- ---- ----     Sprites X Offset
 *
 ************************************************************/
READ16_MEMBER(imagetek_i4100_device::sprite_xoffset_r) { return m_sprite_xoffset; }
WRITE16_MEMBER(imagetek_i4100_device::sprite_xoffset_w) { COMBINE_DATA(&m_sprite_xoffset); }
READ16_MEMBER(imagetek_i4100_device::sprite_yoffset_r) { return m_sprite_yoffset; }
WRITE16_MEMBER(imagetek_i4100_device::sprite_yoffset_w) { COMBINE_DATA(&m_sprite_yoffset); }

/*************************************************************
 *
 * 8.w  ---- ---- ---- ----     Sprites Color Codes Start
 *
 ************************************************************/
READ16_MEMBER(imagetek_i4100_device::sprite_color_code_r) { return m_sprite_color_code; }
WRITE16_MEMBER(imagetek_i4100_device::sprite_color_code_w) { COMBINE_DATA(&m_sprite_color_code); }

/*************************************************************
 *
 * 10.w ---- ---- --54 ----     Layer 2 Priority (3 backmost, 0 frontmost)
 *      ---- ---- ---- 32--     Layer 1 Priority
 *      ---- ---- ---- --10     Layer 0 Priority
 *
 ************************************************************/

READ16_MEMBER(imagetek_i4100_device::layer_priority_r)
{
	return (m_layer_priority[2]<<4) | (m_layer_priority[1]<<2) | m_layer_priority[0];
}

WRITE16_MEMBER(imagetek_i4100_device::layer_priority_w)
{
	m_layer_priority[2] = (data >> 4) & 3;
	m_layer_priority[1] = (data >> 2) & 3;
	m_layer_priority[0] = (data >> 0) & 3;
	if((data >> 6) != 0)
		logerror("%s warning: layer_priority_w write with %04x %04x\n",this->tag(),data,mem_mask);
}

/*************************************************************
 *
 * 12.w ---- cccc cccc cccc     Background Color
 *
 ************************************************************/

READ16_MEMBER(imagetek_i4100_device::background_color_r)
{
	return m_background_color;
}

WRITE16_MEMBER(imagetek_i4100_device::background_color_w)
{
	COMBINE_DATA(&m_background_color);

	m_background_color &= 0x0fff;
	if(data & 0xf000)
		logerror("%s warning: background_color_w write with %04x %04x\n",this->tag(),data,mem_mask);
}

/***************************************************************************
 *
 *  0.w                                 Sprite Y center point
 *  2.w                                 Sprite X center point
 *
 * Appears to apply only for sprites, maybe they applies to tilemaps too under
 * certain conditions
 *
 ***************************************************************************/
READ16_MEMBER(imagetek_i4100_device::screen_xoffset_r) { return m_screen_xoffset; }
WRITE16_MEMBER(imagetek_i4100_device::screen_xoffset_w) { COMBINE_DATA(&m_screen_xoffset); }
READ16_MEMBER(imagetek_i4100_device::screen_yoffset_r) { return m_screen_yoffset; }
WRITE16_MEMBER(imagetek_i4100_device::screen_yoffset_w) { COMBINE_DATA(&m_screen_yoffset); }

READ16_MEMBER(imagetek_i4100_device::window_r) { return m_window[offset]; }
WRITE16_MEMBER(imagetek_i4100_device::window_w) { COMBINE_DATA(&m_window[offset]); }
READ16_MEMBER(imagetek_i4100_device::scroll_r) { return m_scroll[offset]; }
WRITE16_MEMBER(imagetek_i4100_device::scroll_w) { COMBINE_DATA(&m_scroll[offset]); }

/****************************************************
 *
 * Screen Control Register:
 *
 * f--- ---- ---- ----     ?
 * -edc b--- ---- ----
 * ---- -a98 ---- ----     ? Leds (see gakusai attract)
 * ---- ---- 765- ----     16x16 Tiles  (Layer 2-1-0)
 * ---- ---- ---4 32--
 * ---- ---- ---- --1-     Blank Screen
 * ---- ---- ---- ---0     Flip  Screen
 *
 ****************************************************/
WRITE16_MEMBER(imagetek_i4100_device::screen_ctrl_w)
{
	m_layer_tile_select[2] = BIT(data,7);
	m_layer_tile_select[1] = BIT(data,6);
	m_layer_tile_select[0] = BIT(data,5);

	// TODO: some of these must be externalized
	m_screen_blank = BIT(data,1);
	m_screen_flip = BIT(data,0);

	if(data & 0xff1c)
		logerror("%s warning: screen_ctrl_w write with %04x %04x\n",this->tag(),data,mem_mask);

}


/*
    The main CPU has access to the ROMs that hold the graphics through
    a banked window of 64k. Those ROMs also usually store the tables for
    the virtual tiles set. The tile codes to be written to the tilemap
    memory to render the backgrounds are also stored here, in a format
    that the blitter can readily use (which is a form of compression)
*/

READ16_MEMBER(imagetek_i4100_device::gfxrom_r)
{
	offset = offset * 2 + 0x10000 * (m_rombank);

	if (offset < m_gfxrom_size)
		return ((m_gfxrom[offset + 0] << 8) + m_gfxrom[offset + 1]);
	else
		return 0xffff;
}

WRITE16_MEMBER( imagetek_i4100_device::rombank_w )
{
	COMBINE_DATA(&m_rombank);
}

WRITE16_MEMBER( imagetek_i4100_device::crtc_horz_w )
{
	if(m_crtc_unlock == true)
	{
		//logerror("%s CRTC horizontal %04x %04x\n",this->tag(),data,mem_mask);
	}
}

WRITE16_MEMBER( imagetek_i4100_device::crtc_vert_w )
{
	if(m_crtc_unlock == true)
	{
		//logerror("%s CRTC vertical %04x %04x\n",this->tag(),data,mem_mask);
	}
}

WRITE16_MEMBER( imagetek_i4100_device::crtc_unlock_w )
{
	m_crtc_unlock = BIT(data,0);
	if(data & ~1)
		logerror("%s warning: unlock register write with %04x %04x\n",this->tag(),data,mem_mask);
}

/***************************************************************************


                                    Blitter

    [ Registers ]

        Offset:     Value:

        0.l         Destination Tilemap      (1,2,3)
        4.l         Blitter Data Address     (byte offset into the gfx ROMs)
        8.l         Destination Address << 7 (byte offset into the tilemap)

        The Blitter reads a byte and looks at the most significative
        bits for the opcode, while the remaining bits define a value
        (usually how many bytes to write). The opcode byte may be
        followed by a number of other bytes:

            76------            Opcode
            --543210            N
            (at most N+1 bytes follow)


        The blitter is designed to write every other byte (e.g. it
        writes a byte and skips the next). Hence 2 blits are needed
        to fill a tilemap (first even, then odd addresses)

    [ Opcodes ]

            0       Copy the following N+1 bytes. If the whole byte
                    is $00: stop and generate an IRQ

            1       Fill N+1 bytes with a sequence, starting with
                    the  value in the following byte

            2       Fill N+1 bytes with the value in the following
                    byte

            3       Skip N+1 bytes. If the whole byte is $C0:
                    skip to the next row of the tilemap (+0x200 bytes)
                    but preserve the column passed at the start of the
                    blit (destination address % 0x200)


***************************************************************************/

void imagetek_i4100_device::blt_write( address_space &space, const int tmap, const offs_t offs, const uint16_t data, const uint16_t mask )
{
	switch(tmap)
	{
		case 1: vram_0_w(space, offs, data, mask);    break;
		case 2: vram_1_w(space, offs, data, mask);    break;
		case 3: vram_2_w(space, offs, data, mask);    break;
	}
//  logerror("%s : Blitter %X] %04X <- %04X & %04X\n", machine().describe_context(), tmap, offs, data, mask);
}

/***************************************************************************


                                    Blitter

    [ Registers ]

        Offset:     Value:

        0.l         Destination Tilemap      (1,2,3)
        4.l         Blitter Data Address     (byte offset into the gfx ROMs)
        8.l         Destination Address << 7 (byte offset into the tilemap)

        The Blitter reads a byte and looks at the most significative
        bits for the opcode, while the remaining bits define a value
        (usually how many bytes to write). The opcode byte may be
        followed by a number of other bytes:

            76------            Opcode
            --543210            N
            (at most N+1 bytes follow)


        The blitter is designed to write every other byte (e.g. it
        writes a byte and skips the next). Hence 2 blits are needed
        to fill a tilemap (first even, then odd addresses)

    [ Opcodes ]

            0       Copy the following N+1 bytes. If the whole byte
                    is $00: stop and generate an IRQ

            1       Fill N+1 bytes with a sequence, starting with
                    the  value in the following byte

            2       Fill N+1 bytes with the value in the following
                    byte

            3       Skip N+1 bytes. If the whole byte is $C0:
                    skip to the next row of the tilemap (+0x200 bytes)
                    but preserve the column passed at the start of the
                    blit (destination address % 0x200)


***************************************************************************/


// TODO: clean this up
WRITE16_MEMBER( imagetek_i4100_device::blitter_w )
{
	COMBINE_DATA(&m_blitter_regs[offset]);

	if (offset == 0x0c / 2)
	{
		//uint8_t *src     = memregion(DEVICE_SELF)->base();

		uint32_t tmap     = (m_blitter_regs[0x00 / 2] << 16) + m_blitter_regs[0x02 / 2];
		uint32_t src_offs = (m_blitter_regs[0x04 / 2] << 16) + m_blitter_regs[0x06 / 2];
		uint32_t dst_offs = (m_blitter_regs[0x08 / 2] << 16) + m_blitter_regs[0x0a / 2];

		int shift   = (dst_offs & 0x80) ? 0 : 8;
		uint16_t mask = (dst_offs & 0x80) ? 0x00ff : 0xff00;

//      logerror("%s Blitter regs %08X, %08X, %08X\n", machine().describe_context(), tmap, src_offs, dst_offs);

		dst_offs >>= 7 + 1;
		switch (tmap)
		{
			case 1:
			case 2:
			case 3:
				break;
			default:
				logerror("%s Blitter unknown destination: %08X\n", machine().describe_context(), tmap);
				return;
		}

		while (1)
		{
			uint16_t b1, b2, count;

			src_offs %= m_gfxrom_size;
			b1 = m_gfxrom[src_offs];
//          logerror("%s Blitter opcode %02X at %06X\n", machine().describe_context(), b1, src_offs);
			src_offs++;

			count = ((~b1) & 0x3f) + 1;

			switch ((b1 & 0xc0) >> 6)
			{
			case 0:
				/* Stop and Generate an IRQ. We can't generate it now
				       both because it's unlikely that the blitter is so
				       fast and because some games (e.g. lastfort) need to
				       complete the blitter irq service routine before doing
				       another blit. */
				if (b1 == 0)
				{
					m_blit_done_timer->adjust(attotime::from_usec(500));
					return;
				}

				/* Copy */
				while (count--)
				{
					src_offs %= m_gfxrom_size;
					b2 = m_gfxrom[src_offs] << shift;
					src_offs++;

					dst_offs &= 0xffff;
					blt_write(space, tmap, dst_offs, b2, mask);
					dst_offs = ((dst_offs + 1) & (0x100 - 1)) | (dst_offs & (~(0x100 - 1)));
				}
				break;

			case 1:
				/* Fill with an increasing value */
				src_offs %= m_gfxrom_size;
				b2 = m_gfxrom[src_offs];
				src_offs++;

				while (count--)
				{
					dst_offs &= 0xffff;
					blt_write(space, tmap, dst_offs, b2 << shift, mask);
					dst_offs = ((dst_offs + 1) & (0x100 - 1)) | (dst_offs & (~(0x100 - 1)));
					b2++;
				}
				break;

			case 2:
				/* Fill with a fixed value */
				src_offs %= m_gfxrom_size;
				b2 = m_gfxrom[src_offs] << shift;
				src_offs++;

				while (count--)
				{
					dst_offs &= 0xffff;
					blt_write(space, tmap, dst_offs, b2, mask);
					dst_offs = ((dst_offs + 1) & (0x100 - 1)) | (dst_offs & (~(0x100 - 1)));
				}
				break;

			case 3:
				/* Skip to the next line ?? */
				if (b1 == 0xc0)
				{
					dst_offs +=   0x100;
					dst_offs &= ~(0x100 - 1);
					dst_offs |=  (0x100 - 1) & (m_blitter_regs[0x0a / 2] >> (7 + 1));
				}
				else
				{
					dst_offs += count;
				}
				break;

			default:
				//logerror("%s Blitter unknown opcode %02X at %06X\n",machine().describe_context(),b1,src_offs-1);
				return;
			}

		}
	}
}



/***************************************************************************
 *
 * Screen Drawing
 *
 ***************************************************************************/

void imagetek_i4100_device::draw_spritegfx(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &clip,
							uint32_t const gfxstart, uint16_t const width, uint16_t const height,
							uint16_t color, int const flipx, int const flipy, int sx, int sy,
							uint32_t const scale, uint8_t const prival )
{
	if (!scale) return;
	uint8_t trans;
	bool const is_8bpp = (m_support_8bpp == true && color == 0xf) ? true : false;  /* 8bpp */

	if (is_8bpp)
	{
		trans = 0xff;
		color = 0;
		/* Bounds checking */
		if ((gfxstart + width * height - 1) >= m_gfxrom.bytes())
			return;
	}
	else
	{
		trans = 0xf;
		color <<= 4;
		/* Bounds checking */
		if ((gfxstart + width / 2 * height - 1) >= m_gfxrom.bytes())
			return;
	}

	const pen_t *pal = &m_palette->pen((m_sprite_color_code & 0x0f) << 8);
	const uint8_t *source_base;
	if (is_8bpp)
		source_base = &m_gfxrom[gfxstart % m_gfxrom.bytes()];
	else
		source_base = &m_expanded_gfx1[(gfxstart % m_gfxrom.bytes()) << 1];

	int const sprite_screen_height = (scale * height + 0x8000) >> 16;
	int const sprite_screen_width = (scale * width + 0x8000) >> 16;
	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (width << 16) / sprite_screen_width;
		int dy = (height << 16) / sprite_screen_height;

		int ex = sx + sprite_screen_width;
		int ey = sy + sprite_screen_height;

		int x_index_base;
		int y_index;

		if (flipx)
		{
			x_index_base = (sprite_screen_width - 1) * dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if (flipy)
		{
			y_index = (sprite_screen_height - 1) * dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if (sx < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x - sx;
			sx += pixels;
			x_index_base += pixels * dx;
		}
		if (sy < clip.min_y)
		{ /* clip top */
			int pixels = clip.min_y - sy;
			sy += pixels;
			y_index += pixels * dy;
		}
		if (ex > clip.max_x + 1)
		{ /* clip right */
			int pixels = ex - clip.max_x - 1;
			ex -= pixels;
		}
		if (ey > clip.max_y + 1)
		{ /* clip bottom */
			int pixels = ey - clip.max_y - 1;
			ey -= pixels;
		}

		if (ex > sx)
		{ /* skip if inner loop doesn't draw anything */
			int y;
			bitmap_ind8 &priority_bitmap = screen.priority();
			if (priority_bitmap.valid())
			{
				for (y = sy; y < ey; y++)
				{
					const uint8_t *source = source_base + (y_index >> 16) * width;
					uint32_t *dest = &bitmap.pix32(y);
					uint8_t *pri = &priority_bitmap.pix8(y);
					int x, x_index = x_index_base;
					{
						for (x = sx; x < ex; x++)
						{
							uint8_t const c = source[x_index >> 16];

							if (c != trans)
							{
								if (pri[x] <= prival)
									dest[x] = pal[color + c];

								pri[x] = 0xff;
							}
							x_index += dx;
						}
						y_index += dy;
					}
				}
			}
		}
	}
}

 /***************************************************************************


                                Sprites Drawing


        Offset:     Bits:                   Value:

        0.w         fedc b--- ---- ----     Priority (0 = Max)
                    ---- -a98 7654 3210     X

        2.w         fedc ba-- ---- ----     Zoom (Both X & Y)
                    ---- --98 7654 3210     Y

        4.w         f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc b--- ---- ----     Size X *
                    ---- -a98 ---- ----     Size Y *
                    ---- ---- 7654 ----     Color
                    ---- ---- ---- 3210     Code High Bits **

        6.w                                 Code Low Bits  **

*  8 pixel increments
** 8x8 pixel increments

***************************************************************************/

void imagetek_i4100_device::draw_sprites( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	int max_x = (m_screen_xoffset + 1) * 2;
	int max_y = (m_screen_yoffset + 1) * 2;
	int m_sprite_xoffs = m_sprite_xoffset - (m_screen_xoffset + 1);
	int m_sprite_yoffs = m_sprite_yoffset - (m_screen_yoffset + 1);

	int max_sprites = m_spriteram->bytes() / 8;
	int sprites     = m_sprite_count % max_sprites;

	// Exponential zoom table extracted from daitoride
	static const uint16_t zoomtable[0x40] =
	{   0xAAC,0x800,0x668,0x554,0x494,0x400,0x390,0x334,
		0x2E8,0x2AC,0x278,0x248,0x224,0x200,0x1E0,0x1C8,
		0x1B0,0x198,0x188,0x174,0x164,0x154,0x148,0x13C,
		0x130,0x124,0x11C,0x110,0x108,0x100,0x0F8,0x0F0,
		0x0EC,0x0E4,0x0DC,0x0D8,0x0D4,0x0CC,0x0C8,0x0C4,
		0x0C0,0x0BC,0x0B8,0x0B4,0x0B0,0x0AC,0x0A8,0x0A4,
		0x0A0,0x09C,0x098,0x094,0x090,0x08C,0x088,0x080,
		0x078,0x070,0x068,0x060,0x058,0x050,0x048,0x040 };

	uint16_t *src;
	int inc;

	if (sprites == 0)
		return;

	for (int i = 0; i < 0x20; i++)
	{
		if (!(m_sprite_priority & 0x8000))
		{
			src = (m_spriteram_buffered ? m_spriteram->buffer() : m_spriteram->live()) + (sprites - 1) * (8 / 2);
			inc = -(8 / 2);
		}
		else
		{
			src = (m_spriteram_buffered ? m_spriteram->buffer() : m_spriteram->live());
			inc = (8 / 2);
		}

		for (int j = 0; j < sprites; j++)
		{
			int x = src[0];
			int const curr_pri = (x & 0xf800) >> 11;

			if ((curr_pri == 0x1f) || (curr_pri != i))
			{
				src += inc;
				continue;
			}

			uint8_t pri = (m_sprite_priority & 0x0300) >> 8;

			if (!(m_sprite_priority & 0x8000))
			{
				if (curr_pri > (m_sprite_priority & 0x1f))
					pri = (m_sprite_priority & 0x0c00) >> 10;
			}

			int y               = src[1];
			uint16_t const attr = src[2];
			uint16_t const code = src[3];

			int flipx            = attr & 0x8000;
			int flipy            = attr & 0x4000;
			uint16_t const color = (attr & 0xf0) >> 4;

			uint32_t const zoom = zoomtable[(y & 0xfc00) >> 10] << (16 - 8);

			x = (x & 0x07ff) - m_sprite_xoffs;
			y = (y & 0x03ff) - m_sprite_yoffs;

			int const width  = (((attr >> 11) & 0x7) + 1) * 8;
			int const height = (((attr >>  8) & 0x7) + 1) * 8;

			uint32_t const gfxstart = (8 * 8 * 4 / 8) * (((attr & 0x000f) << 16) + code);

			if (m_screen_flip)
			{
				flipx = !flipx;     x = max_x - x - width;
				flipy = !flipy;     y = max_y - y - height;
			}

			draw_spritegfx(screen, bitmap, cliprect, gfxstart, width, height, color, flipx, flipy, x, y, zoom, 3 - pri);

			src += inc;
		}
	}
}


inline uint8_t imagetek_i4100_device::get_tile_pix( uint16_t code, uint8_t x, uint8_t y, bool const big, uint32_t &pix)
{
	// Use code as an index into the tiles set table
	int table_index = (code & 0x1ff0) >> 3;
	uint32_t tile = (m_tiletable[table_index] << 16) + m_tiletable[table_index | 1];

	if (code & 0x8000) // Special: draw a tile of a single color (i.e. not from the gfx ROMs)
	{
		pix = m_palette->pen(code & 0x0fff);

		if ((code & 0xf) != 0xf)
			return 1;
		else
			return 0;
	}
	else if (((tile & 0x00f00000) == 0x00f00000) && (m_support_8bpp == true)) /* draw tile as 8bpp (e.g. balcube bg) */
	{
		gfx_element *gfx1 = gfx(big ? 3 : 1);
		uint32_t tile2 = big ? ((tile & 0xfffff) + ((code & 0xf) << 3)) :
								((tile & 0xfffff) + ((code & 0xf) << 1));
		const uint16_t* data;
		uint8_t flipxy = (code & 0x6000) >> 13;

		if (tile2 < gfx1->elements())
			data = gfx1->get_data(tile2);
		else
		{
			pix = rgb_t::black();
			return 0;
		}

		uint32_t idx = 0;
		switch (flipxy)
		{
			default:
			case 0x0: idx = (y                    * (big ? 16 : 8)) + x;                    break;
			case 0x1: idx = (((big ? 15 : 7) - y) * (big ? 16 : 8)) + x;                    break;
			case 0x2: idx = (y                    * (big ? 16 : 8)) + ((big ? 15 : 7) - x); break;
			case 0x3: idx = (((big ? 15 : 7) - y) * (big ? 16 : 8)) + ((big ? 15 : 7) - x); break;
		}

		pix = m_palette->pen(data[idx] | (tile & 0x0f000000) >> 16);

		if ((data[idx] & 0xff) != 0xff)
			return 1;
		else
			return 0;
	}
	else
	{
		gfx_element *gfx1 = gfx(big ? 2 : 0);
		uint32_t tile2 = big ? ((tile & 0xfffff) + ((code & 0xf) << 2)) :
								((tile & 0xfffff) + (code & 0xf));
		const uint16_t* data;
		uint8_t flipxy = (code & 0x6000) >> 13;

		if (tile2 < gfx1->elements())
			data = gfx1->get_data(tile2);
		else
		{
			pix = rgb_t::black();
			return 0;
		}

		uint32_t idx = 0;
		switch (flipxy)
		{
			default:
			case 0x0: idx = (y                    * (big ? 16 : 8)) + x;                    break;
			case 0x1: idx = (((big ? 15 : 7) - y) * (big ? 16 : 8)) + x;                    break;
			case 0x2: idx = (y                    * (big ? 16 : 8)) + ((big ? 15 : 7) - x); break;
			case 0x3: idx = (((big ? 15 : 7) - y) * (big ? 16 : 8)) + ((big ? 15 : 7) - x); break;
		}

		pix = m_palette->pen(data[idx] | (tile & 0x0ff00000) >> 16);

		if ((data[idx] & 0xf) != 0xf)
			return 1;
		else
			return 0;
	}
}

/***************************************************************************

                        Tilemaps: Tiles Set & Window

    Each entry in the Tiles Set RAM uses 2 words to specify a starting
    tile code and a color code. This adds 16 consecutive tiles with
    that color code to the set of available tiles.

        Offset:     Bits:                   Value:

        0.w         fedc ---- ---- ----
                    ---- ba98 7654 ----     Color Code*
                    ---- ---- ---- 3210     Code High Bits

        2.w                                 Code Low Bits

* 00-ff, but on later chips supporting it, xf means 256 color tile and palette x

***************************************************************************/

void imagetek_i4100_device::draw_tilemap( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t flags, uint32_t const pcode,
							int sx, int sy, int wx, int wy, bool const big, uint16_t const *tilemapram, int const layer )
{
	int y;

	bitmap_ind8 &priority_bitmap = screen.priority();

	int width  = big ? 4096 : 2048;
	int height = big ? 4096 : 2048;

	int scrwidth  = bitmap.width();
	int scrheight = bitmap.height();

	int windowwidth  = width >> 2;
	int windowheight = height >> 3;

	int dx = m_tilemap_scrolldx[layer] * (m_screen_flip ? 1 : -1);
	int dy = m_tilemap_scrolldy[layer] * (m_screen_flip ? 1 : -1);

	sx += dx;
	sy += dy;

	int min_x, max_x, min_y, max_y;

	if (dx != 0)
	{
		min_x = 0;
		max_x = scrwidth - 1;
	}
	else
	{
		min_x = cliprect.min_x;
		max_x = cliprect.max_x;
	}

	if (dy != 0)
	{
		min_y = 0;
		max_y = scrheight - 1;
	}
	else
	{
		min_y = cliprect.min_y;
		max_y = cliprect.max_y;
	}

	for (y = min_y; y <= max_y; y++)
	{
		int scrolly = (sy + y - wy) & (windowheight - 1);
		int x;
		uint8_t *priority_baseaddr;
		int srcline = (wy + scrolly) & (height - 1);
		int srctilerow = srcline >> (big ? 4 : 3);

		if (!m_screen_flip)
		{
			uint32_t *dst = &bitmap.pix32(y);
			priority_baseaddr = &priority_bitmap.pix8(y);

			for (x = min_x; x <= max_x; x++)
			{
				int scrollx = (sx + x - wx) & (windowwidth - 1);
				int srccol = (wx + scrollx) & (width - 1);
				int srctilecol = srccol >> (big ? 4 : 3);
				int tileoffs = srctilecol + srctilerow * BIG_NX;

				uint32_t dat = 0;

				uint16_t tile = tilemapram[tileoffs];
				uint8_t draw = get_tile_pix(tile, big ? (srccol & 0xf) : (srccol & 0x7), big ? (srcline & 0xf) : (srcline & 0x7), big, dat);

				if (draw)
				{
					dst[x] = dat;
					priority_baseaddr[x] = (priority_baseaddr[x] & (pcode >> 8)) | pcode;
				}
			}
		}
		else // flipped case
		{
			uint32_t *dst = &bitmap.pix32(scrheight-y-1);
			priority_baseaddr = &priority_bitmap.pix8(scrheight-y-1);

			for (x = min_x; x <= max_x; x++)
			{
				int scrollx = (sx + x - wx) & (windowwidth-1);
				int srccol = (wx + scrollx) & (width - 1);
				int srctilecol = srccol >> (big ? 4 : 3);
				int tileoffs = srctilecol + srctilerow * BIG_NX;

				uint32_t dat = 0;

				uint16_t tile = tilemapram[tileoffs];
				uint8_t draw = get_tile_pix(tile, big ? (srccol & 0xf) : (srccol & 0x7), big ? (srcline & 0xf) : (srcline & 0x7), big, dat);

				if (draw)
				{
					dst[scrwidth-x-1] = dat;
					priority_baseaddr[scrwidth-x-1] = (priority_baseaddr[scrwidth-x-1] & (pcode >> 8)) | pcode;
				}
			}
		}
	}
}


void imagetek_i4100_device::draw_layers( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri )
{
	// Draw all the layers with priority == pri
	for (int layer = 2; layer >= 0; layer--)
	{
		if (pri == m_layer_priority[layer])
		{
			// Scroll and Window values
			uint16_t sy = m_scroll[layer * 2 + 0]; uint16_t sx = m_scroll[layer * 2 + 1];
			uint16_t wy = m_window[layer * 2 + 0]; uint16_t wx = m_window[layer * 2 + 1];

			uint16_t *tilemapram = nullptr;

			switch (layer)
			{
				case 0: tilemapram = m_vram_0;   break;
				case 1: tilemapram = m_vram_1;   break;
				case 2: tilemapram = m_vram_2;   break;
			}

			bool const big = (m_support_16x16 && m_layer_tile_select[layer]) == 1;

			draw_tilemap(screen, bitmap, cliprect, 0, 3 - pri, sx, sy, wx, wy, big, tilemapram, layer);
		}
	}
}


void imagetek_i4100_device::draw_foreground(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	if (m_screen_blank == true)
		return;

	for (int pri = 3; pri >= 0; pri--)
		draw_layers(screen, bitmap, cliprect, pri);

	draw_sprites(screen, bitmap, cliprect);
}

uint32_t imagetek_i4100_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(m_background_color), cliprect);

	draw_foreground(screen, bitmap, cliprect);

	return 0;
}

WRITE_LINE_MEMBER(imagetek_i4100_device::screen_eof)
{
	if (state)
	{
		if (!m_spriteram_buffered)
			return;

		m_spriteram->copy();
	}
}
