// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia
/*

IGS017 / IGS031 video device

what's the difference between IGS017 and IGS031? encryption?

all the known IGS017 / IGS031 games use same memory map, is the IGS017 / IGS031
providing the interface to the 8255, or is it coincidence?

*/

#include "emu.h"
#include "igs017_igs031.h"

void igs017_igs031_device::map(address_map &map)
{
	map(0x1000, 0x17ff).ram().share("spriteram");
//  map(0x1800, 0x1bff).ram() //.w("palette", FUNC(palette_device::write).share("palette");
	map(0x1800, 0x1bff).ram().w(FUNC(igs017_igs031_device::palram_w)).share("palram");
	map(0x1c00, 0x1fff).ram();

	map(0x2010, 0x2013).r(FUNC(igs017_igs031_device::i8255_r));
	map(0x2012, 0x2012).w(FUNC(igs017_igs031_device::video_disable_w));

	map(0x2014, 0x2014).w(FUNC(igs017_igs031_device::nmi_enable_w));
	map(0x2015, 0x2015).w(FUNC(igs017_igs031_device::irq_enable_w));

	map(0x4000, 0x5fff).ram().w(FUNC(igs017_igs031_device::fg_w)).share("fg_videoram");
	map(0x6000, 0x7fff).ram().w(FUNC(igs017_igs031_device::bg_w)).share("bg_videoram");

}

u8 igs017_igs031_device::i8255_r(offs_t offset)
{
	if (m_i8255)
		return m_i8255->read(offset);

	logerror("igs017_igs031_device::i8255_r(%02x) with no 8255 device\n", offset);
	return 0;
}


static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8*3,8*2,8*1,8*0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*4) },
	8*8*4
};

GFXDECODE_MEMBER( igs017_igs031_device::gfxinfo )
	GFXDECODE_DEVICE( "tilemaps", 0, layout_8x8x4, 0, 16 )
//  GFXDECODE_DEVICE( "sprites",  0, spritelayout, 0,  8 )
GFXDECODE_END


DEFINE_DEVICE_TYPE(IGS017_IGS031, igs017_igs031_device, "igs017_031", "IGS017_IGS031")

igs017_igs031_device::igs017_igs031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS017_IGS031, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo, "palette")
	, device_video_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_palette_scramble_cb(*this, FUNC(igs017_igs031_device::palette_callback_straight))
	, m_space_config("igs017_igs031", ENDIANNESS_BIG, 8,15, 0, address_map_constructor(FUNC(igs017_igs031_device::map), this))
	, m_spriteram(*this, "spriteram", 0)
	, m_fg_videoram(*this, "fg_videoram", 0)
	, m_bg_videoram(*this, "bg_videoram", 0)
	, m_palram(*this, "palram", 0)
	, m_i8255(*this, finder_base::DUMMY_TAG)
	, m_palette(*this, "palette")
	, m_revbits(false)
{
}

device_memory_interface::space_config_vector igs017_igs031_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

u16 igs017_igs031_device::palette_callback_straight(u16 bgr) const
{
	return bgr;
}

void igs017_igs031_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x400/2);
}

void igs017_igs031_device::device_start()
{
	m_palette_scramble_cb.resolve();

	m_fg_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(igs017_igs031_device::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,32);
	m_bg_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(igs017_igs031_device::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,32);

	m_fg_tilemap->set_transparent_pen(0xf);
	m_bg_tilemap->set_transparent_pen(0xf);

	m_toggle = 0;
	m_debug_addr = 0;
	m_debug_width = 512;

	save_item(NAME(m_video_disable));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_irq_enable));
}

void igs017_igs031_device::video_start()
{
	// make sure this happens AFTER driver init, or things won't work
	expand_sprites();

	if (m_revbits)
	{
		u8 *rom        = memregion("tilemaps")->base();
		const u32 size = memregion("tilemaps")->bytes();

		for (int i = 0; i < size ; i++)
		{
			rom[i] = bitswap<8>(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);
//          rom[i^1] = bitswap<8>(rom[i], 0, 1, 2, 3, 4, 5, 6, 7);
		}
	}
}

void igs017_igs031_device::device_reset()
{
	m_video_disable = false;
	m_nmi_enable = false;
	m_irq_enable = false;
}

void igs017_igs031_device::write(offs_t offset, u8 data)
{
	space().write_byte(offset, data);
}

u8 igs017_igs031_device::read(offs_t offset)
{
	return space().read_byte(offset);
}


void igs017_igs031_device::video_disable_w(u8 data)
{
	m_video_disable = data & 1;
	if (data & (~1))
		logerror("%s: unknown bits of video_disable written = %02x\n", machine().describe_context(), data);
//  popmessage("VIDEO %02X",data);
}

void igs017_igs031_device::palram_w(offs_t offset, u8 data)
{
	m_palram[offset] = data;

	offset &= ~1;

	u16 bgr = (m_palram[offset+1] << 8) | (m_palram[offset]);

	// bitswap (some games)
	bgr = m_palette_scramble_cb(bgr);

	m_palette->set_pen_color(offset/2, pal5bit(bgr >> 0), pal5bit(bgr >> 5), pal5bit(bgr >> 10));

}

static inline const u32 COLOR(u16 x) { return (x >> 2) & 7; }

TILE_GET_INFO_MEMBER(igs017_igs031_device::get_fg_tile_info)
{
	const u16 code = m_fg_videoram[tile_index * 4 + 0] + (m_fg_videoram[tile_index * 4 + 1] << 8);
	const u16 attr = m_fg_videoram[tile_index * 4 + 2] + (m_fg_videoram[tile_index * 4 + 3] << 8);
	SET_TILE_INFO_MEMBER(0, code, COLOR(attr), TILE_FLIPXY(attr >> 5));
}
TILE_GET_INFO_MEMBER(igs017_igs031_device::get_bg_tile_info)
{
	const u16 code = m_bg_videoram[tile_index * 4 + 0] + (m_bg_videoram[tile_index * 4 + 1] << 8);
	const u16 attr = m_bg_videoram[tile_index * 4 + 2] + (m_bg_videoram[tile_index * 4 + 3] << 8);
	SET_TILE_INFO_MEMBER(0, code, COLOR(attr)+8, TILE_FLIPXY(attr >> 5));
}

void igs017_igs031_device::fg_w(offs_t offset, u8 data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset/4);
}

void igs017_igs031_device::bg_w(offs_t offset, u8 data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset/4);
}


// Eeach 16 bit word in the sprites gfx roms contains three 5 bit pens: x-22222-11111-00000 (little endian!).
// This routine expands each word into three bytes.
void igs017_igs031_device::expand_sprites()
{
	u8 *rom        = memregion("sprites")->base();
	const u32 size = memregion("sprites")->bytes();

	m_sprites_gfx_size   =   size / 2 * 3;
	m_sprites_gfx        =   std::make_unique<u8[]>(m_sprites_gfx_size);

	for (int i = 0; i < size / 2; i++)
	{
		const u16 pens = (rom[i*2+1] << 8) | rom[i*2];

		m_sprites_gfx[i * 3 + 0] = (pens >>  0) & 0x1f;
		m_sprites_gfx[i * 3 + 1] = (pens >>  5) & 0x1f;
		m_sprites_gfx[i * 3 + 2] = (pens >> 10) & 0x1f;
	}
}

/***************************************************************************
                              Sprites Format

    Offset:         Bits:               Value:

        0.b                             Y (low)

        1.b         7654 32--           Size Y (low)
                    ---- --10           Y (high)

        2.b         7654 3---           X (low)
                    ---- -210           Size Y (high)

        3.b         76-- ----           Size X (low)
                    --5- ----
                    ---4 3210           X (high)

        4.b         76-- ----           Code (low)
                    --54 3210           Size X (high)

        5.b                             Code (mid low)

        6.b                             Code (mid high)

        7.b         765- ----           Color
                    ---4 ----           Flip X
                    ---- 3---
                    ---- -210           Code (high)

    Code = ROM Address / 2 = Pixel / 3

***************************************************************************/

void igs017_igs031_device::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int offsx, int offsy, int dimx, int dimy, int flipx, int flipy, u32 color, u32 addr)
{
	// Bounds checking
	if (addr + dimx * dimy >= m_sprites_gfx_size)
		return;

	/* Start drawing */
	const u16 pal = 0x100 + (color << 5);
	const u8 *source_base = &m_sprites_gfx[addr];
	const u8 transparent_color = 0x1f;

	int xinc = flipx ? -1 : 1;
	int yinc = flipy ? -1 : 1;

	int x_index_base = flipx ? dimx - 1 : 0;
	int y_index = flipy ? dimy - 1 : 0;

	// start coordinates
	int sx = offsx;
	int sy = offsy;

	// end coordinates
	int ex = sx + dimx;
	int ey = sy + dimy;

	if (sx < cliprect.min_x)
	{ // clip left
		int pixels = cliprect.min_x - sx;
		sx += pixels;
		x_index_base += xinc * pixels;
	}
	if (sy < cliprect.min_y)
	{ // clip top
		int pixels = cliprect.min_y - sy;
		sy += pixels;
		y_index += yinc * pixels;
	}
	// NS 980211 - fixed incorrect clipping
	if (ex > cliprect.max_x + 1)
	{ // clip right
		ex = cliprect.max_x + 1;
	}
	if (ey > cliprect.max_y + 1)
	{ // clip bottom
		ey = cliprect.max_y + 1;
	}

	if (ex > sx)
	{ // skip if inner loop doesn't draw anything
		for (int y = sy; y < ey; y++)
		{
			const u8 *source = source_base + y_index * dimx;
			u16 *dest = &bitmap.pix16(y);
			int x_index = x_index_base;
			for (int x = sx; x < ex; x++)
			{
				const u8 c = source[x_index];
				if (c != transparent_color)
					dest[x] = pal + c;

				x_index += xinc;
			}
			y_index += yinc;
		}
	}
}

void igs017_igs031_device::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	const u8 *s    =   m_spriteram;
	const u8 *end  =   m_spriteram + 0x800;

	for (; s < end; s += 8)
	{
		const int y     =   s[0] + (s[1] << 8);
		int x           =   s[2] + (s[3] << 8);
		u32 addr        =   (s[4] >> 6) | (s[5] << 2) | (s[6] << 10) | ((s[7] & 0x07) << 18);
		addr            *=  3;

		const int flipx =   s[7] & 0x10;
		const int flipy =   0;

		const int dimx  =   ((((s[4] & 0x3f)<<2) | ((s[3] & 0xc0)>>6))+1) * 3;
		const int dimy  =   ((y >> 10) | ((x & 0x03)<<6))+1;

		x               >>= 3;
		const int sx    =   (x & 0x1ff) - (x & 0x200);
		const int sy    =   (y & 0x1ff) - (y & 0x200);

		// sprites list stop (used by mgdh & sdmg2 during don den)
		if (sy == -0x200)
			break;

		const u32 color = (s[7] & 0xe0) >> 5;

		draw_sprite(bitmap, cliprect, sx, sy, dimx, dimy, flipx, flipy, color, addr);
	}
}

// A simple gfx viewer (toggle with T)
int igs017_igs031_device::debug_viewer(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_T))   m_toggle ^= 1;
	if (m_toggle)    {
		int h = 256, w = m_debug_width, a = m_debug_addr;

		if (machine().input().code_pressed(KEYCODE_O))        w += 1;
		if (machine().input().code_pressed(KEYCODE_I))        w -= 1;

		if (machine().input().code_pressed(KEYCODE_U))        w += 8;
		if (machine().input().code_pressed(KEYCODE_Y))        w -= 8;

		if (machine().input().code_pressed(KEYCODE_RIGHT))    a += 1;
		if (machine().input().code_pressed(KEYCODE_LEFT))     a -= 1;

		if (machine().input().code_pressed(KEYCODE_DOWN))     a += w;
		if (machine().input().code_pressed(KEYCODE_UP))       a -= w;

		if (machine().input().code_pressed(KEYCODE_PGDN))     a += w * h;
		if (machine().input().code_pressed(KEYCODE_PGUP))     a -= w * h;

		if (a < 0)      a = 0;
		if (a > m_sprites_gfx_size)  a = m_sprites_gfx_size;

		if (w <= 0)     w = 0;
		if (w > 1024)   w = 1024;

		bitmap.fill(0, cliprect);

		draw_sprite(bitmap, cliprect, 0,0, w,h, 0,0, 0, a);

		popmessage("a: %08X w: %03X p: %02x-%02x-%02x",a,w,m_sprites_gfx[a/3*3+0],m_sprites_gfx[a/3*3+1],m_sprites_gfx[a/3*3+2]);
		m_debug_addr = a;
		m_debug_width = w;
		osd_sleep(osd_ticks_per_second() / 1000 * 200);
		return 1;
	}
#endif
	return 0;
}

u32 igs017_igs031_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 2;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	if (debug_viewer(bitmap,cliprect))
		return 0;

	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_video_disable)
		return 0;

	if (layers_ctrl & 1)    m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	if (layers_ctrl & 4)    draw_sprites(bitmap, cliprect);

	if (layers_ctrl & 2)    m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void igs017_igs031_device::nmi_enable_w(u8 data)
{
	m_nmi_enable = data & 1;
	if (data != 0 && data != 1 && data != 0xff)
		logerror("%s: nmi_enable = %02x\n", machine().describe_context(), data);
}

void igs017_igs031_device::irq_enable_w(u8 data)
{
	m_irq_enable = data & 1;
	if (data != 0 && data != 1 && data != 0xff)
		logerror("%s: irq_enable = %02x\n", machine().describe_context(), data);
}
