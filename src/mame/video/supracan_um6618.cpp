// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz,David Haywood

/*
	The UM6619 integrates:

	Video Controller (Tilemaps + Roz Tilemaps + Sprites)
	Palette Controller + RAM
	Sprite DMA Controller
	Interrupt Generation

    Known unemulated graphical effects and issues:
    - All: Sprite sizing is still imperfect.
    - All: Sprites need to be converted to use scanline rendering for proper clipping.
    - All: Improperly-emulated 1bpp ROZ mode, used by the Super A'Can BIOS logo.
    - All: Unimplemented ROZ scaling tables, used by the Super A'Can BIOS logo and Speedy Dragon intro, among others.
    - All: Priorities are largely unknown.
    - C.U.G.: Gameplay backgrounds are broken.
    - Sango Fighter: Possible missing masking on the upper edges of the screen during gameplay.
    - Sango Fighter: Raster effects off by 1 line
    - Sango Fighter: Specifies tiles out of range of video ram??
    - Speedy Dragon: Backgrounds are broken (wrong tile bank/region).
    - Super Taiwanese Baseball League: Does not boot, uses an unemulated DMA type
    - Super Taiwanese Baseball League: Missing window effect applied on tilemaps?
    - The Son of Evil: Many graphical issues.
    - Visible area, looks like it should be 224 pixels high at most, most games need 8 off the top and 8 off the bottom (or a global scroll)
      Sango looks like it needs 16 off the bottom instead
      Visible area is almost certainly 224 as Son of Evil has an explicit check in the vblank handler
*/

#include "emu.h"
#include "video/supracan_um6618.h"

#define DRAW_DEBUG_ROZ          (0)

#define DRAW_DEBUG_UNK_SPRITE   (0)

#define DEBUG_PRIORITY          (0)
#define DEBUG_PRIORITY_INDEX    (0) // 0-3

#define LOG_UNKNOWNS    (1 << 0)
#define LOG_SPRDMA      (1 << 2)
#define LOG_SPRITES     (1 << 3)
#define LOG_TILEMAP0    (1 << 4)
#define LOG_TILEMAP1    (1 << 5)
#define LOG_TILEMAP2    (1 << 6)
#define LOG_ROZ         (1 << 7)
#define LOG_HFVIDEO     (1 << 8)
#define LOG_IRQS        (1 << 9)
#define LOG_HFUNKNOWNS  (1 << 11)
#define LOG_VIDEO       (LOG_SPRDMA | LOG_SPRITES | LOG_TILEMAP0 | LOG_TILEMAP1 | LOG_TILEMAP2 | LOG_ROZ)
#define LOG_DEFAULT     (LOG_ALL & ~(LOG_HFVIDEO | LOG_HFUNKNOWNS))

#define VERBOSE         (LOG_UNKNOWNS)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SUPRACAN_UM6618_VIDEO, supracan_um6618_video_device, "umc6618", "UM6618 Video Controller")


supracan_um6618_video_device::supracan_um6618_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SUPRACAN_UM6618_VIDEO, tag, owner, clock)
	, m_gfxdecode(*this, "gfxdecode")
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_vram(*this, "vram")
	, m_read_maincpu_space(*this)
	, m_write_maincpu_space(*this)
	, m_vblank_irq(*this)
	, m_line_irq(*this)
	, m_hblank_irq(*this)
{
}

void supracan_um6618_video_device::device_start()
{
	m_sprite_final_bitmap.allocate(320, 256, BITMAP_FORMAT_IND16);
	m_sprite_mask_bitmap.allocate(320, 256, BITMAP_FORMAT_IND8);
	m_prio_bitmap.allocate(320, 256, BITMAP_FORMAT_IND8);

	m_vram_addr_swapped.resize(0x20000); // hack for 1bpp layer at startup

	m_tilemap_sizes[0][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[0][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[0][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[0][3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap_sizes[1][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[1][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[1][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[1][3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap_sizes[2][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[2][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[2][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[2][3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_tilemap2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap_sizes[3][0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_roz_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap_sizes[3][1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_roz_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap_sizes[3][2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_roz_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_tilemap_sizes[3][3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supracan_um6618_video_device::get_roz_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	save_item(NAME(m_sprdma_regs.src));
	save_item(NAME(m_sprdma_regs.src_inc));
	save_item(NAME(m_sprdma_regs.dst));
	save_item(NAME(m_sprdma_regs.dst_inc));
	save_item(NAME(m_sprdma_regs.count));
	save_item(NAME(m_sprdma_regs.control));

	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_base_addr));
	save_item(NAME(m_sprite_flags));

	save_item(NAME(m_tilemap_base_addr));
	save_item(NAME(m_tilemap_scrollx));
	save_item(NAME(m_tilemap_scrolly));
	save_item(NAME(m_video_flags));
	save_item(NAME(m_tilemap_flags));
	save_item(NAME(m_tilemap_mode));
	save_item(NAME(m_screen_mode_control));

	save_item(NAME(m_roz_base_addr));
	save_item(NAME(m_roz_mode));
	save_item(NAME(m_roz_scrollx));
	save_item(NAME(m_roz_scrolly));
	save_item(NAME(m_roz_tile_bank));
	save_item(NAME(m_roz_unk_base0));
	save_item(NAME(m_roz_unk_base1));
	save_item(NAME(m_roz_unk_base2));
	save_item(NAME(m_roz_coeffa));
	save_item(NAME(m_roz_coeffb));
	save_item(NAME(m_roz_coeffc));
	save_item(NAME(m_roz_coeffd));
	save_item(NAME(m_roz_changed));
	save_item(NAME(m_unk_1d0));

	save_item(NAME(m_video_regs));

	m_video_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_um6618_video_device::video_callback), this));
	m_hbl_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_um6618_video_device::hbl_callback), this));
	m_line_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_um6618_video_device::line_on_callback), this));
	m_line_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(supracan_um6618_video_device::line_off_callback), this));

	m_read_maincpu_space.resolve();
	m_write_maincpu_space.resolve_safe();
	m_vblank_irq.resolve_safe();
	m_line_irq.resolve_safe();
	m_hblank_irq.resolve_safe();
}

void supracan_um6618_video_device::device_reset()
{
	m_gfxdecode->gfx(4)->set_source(&m_vram_addr_swapped[0]);
	m_gfxdecode->gfx(4)->set_xormask(0);

	m_sprite_count = 0;
	m_sprite_base_addr = 0;
	m_sprite_flags = 0;

	m_video_timer->adjust(m_screen->time_until_pos(0, 0));
	m_screen_mode_control = 0;

	m_roz_base_addr = 0;
	m_roz_mode = 0;
	std::fill(std::begin(m_tilemap_base_addr), std::end(m_tilemap_base_addr), 0);
}

void supracan_um6618_video_device::palette_init(palette_device &palette) const
{
	// Used for debugging purposes for now
	for (int i = 0; i < 32768; i++)
	{
		int const r = (i & 0x1f) << 3;
		int const g = ((i >> 5) & 0x1f) << 3;
		int const b = ((i >> 10) & 0x1f) << 3;
		palette.set_pen_color(i, r, g, b);
	}
}


int supracan_um6618_video_device::get_tilemap_dimensions(int &xsize, int &ysize, int layer)
{
	xsize = 32;
	ysize = 32;

	int select;
	if (layer == 3)
		select = m_roz_mode & 0x0f00;
	else
		select = m_tilemap_flags[layer] & 0x0f00;

	switch (select)
	{
	case 0x600:
		xsize = 64;
		ysize = 32;
		return 1;

	case 0xa00:
		xsize = 128;
		ysize = 32;
		return 2;

	case 0xc00:
		xsize = 64;
		ysize = 64;
		return 3;

	default:
		LOGMASKED(LOG_HFUNKNOWNS, "Unsupported tilemap size for layer %d: %04x\n", layer, select);
		return 0;
	}
}

void supracan_um6618_video_device::draw_sprite_tile(bitmap_ind16 &dst, bitmap_ind8 &priomap, const rectangle &cliprect, gfx_element *gfx, int tile, int palette,
	bool xflip, bool yflip, int dstx, int dsty, int prio)
{
	// compute final pixel in X and exit if we are entirely clipped
	int dstendx = dstx + 7;
	if (dstx > cliprect.right() || dstendx < cliprect.left())
		return;

	// apply left clip
	int srcx = 0;
	if (dstx < cliprect.left())
	{
		srcx = cliprect.left() - dstx;
		dstx = cliprect.left();
	}

	// apply right clip
	if (dstendx > cliprect.right())
		dstendx = cliprect.right();

	// compute final pixel in Y and exit if we are entirely clipped
	int dstendy = dsty + 7;
	if (dsty > cliprect.bottom() || dstendy < cliprect.top())
		return;

	// apply top clip
	int srcy = 0;
	if (dsty < cliprect.top())
	{
		srcy = cliprect.top() - dsty;
		dsty = cliprect.top();
	}

	// apply bottom clip
	if (dstendy > cliprect.bottom())
		dstendy = cliprect.bottom();

	// apply X flipping
	int dx = 1;
	if (xflip)
	{
		srcx = 7 - srcx;
		dx = -dx;
	}

	// apply Y flipping
	int dy = gfx->rowbytes();
	if (yflip)
	{
		srcy = 7 - srcy;
		dy = -dy;
	}

	const int color = gfx->colorbase() + gfx->granularity() * (palette % gfx->colors());
	const uint8_t *src_data = &gfx->get_data(tile % gfx->elements())[srcy * gfx->rowbytes()];
	for (int y = dsty; y <= dstendy; y++)
	{
		const uint8_t *srcp = &src_data[srcx];
		uint8_t *priop = &priomap.pix(y, dstx);
		uint16_t *dstp = &dst.pix(y, dstx);
		for (int x = dstx; x <= dstendx; x++)
		{
			const uint32_t srcdata = *srcp;
			if (srcdata != 0)
			{
				*dstp = (uint16_t)(srcdata + color);
				*priop = (*priop & 0xf0) | (uint8_t)prio;
			}
			srcp += dx;
			priop++;
			dstp++;
		}
		src_data += dy;
	}
}

void supracan_um6618_video_device::draw_sprite_tile_mask(bitmap_ind8 &dst, const rectangle &cliprect, gfx_element *gfx, int tile, bool xflip, bool yflip, int dstx, int dsty)
{
	// compute final pixel in X and exit if we are entirely clipped
	int dstendx = dstx + 7;
	if (dstx > cliprect.right() || dstendx < cliprect.left())
		return;

	// apply left clip
	int srcx = 0;
	if (dstx < cliprect.left())
	{
		srcx = cliprect.left() - dstx;
		dstx = cliprect.left();
	}

	// apply right clip
	if (dstendx > cliprect.right())
		dstendx = cliprect.right();

	// compute final pixel in Y and exit if we are entirely clipped
	int dstendy = dsty + 7;
	if (dsty > cliprect.bottom() || dstendy < cliprect.top())
		return;

	// apply top clip
	int srcy = 0;
	if (dsty < cliprect.top())
	{
		srcy = cliprect.top() - dsty;
		dsty = cliprect.top();
	}

	// apply bottom clip
	if (dstendy > cliprect.bottom())
		dstendy = cliprect.bottom();

	// apply X flipping
	int dx = 1;
	if (xflip)
	{
		srcx = 7 - srcx;
		dx = -dx;
	}

	// apply Y flipping
	int dy = gfx->rowbytes();
	if (yflip)
	{
		srcy = 7 - srcy;
		dy = -dy;
	}

	const uint8_t *src_data = &gfx->get_data(tile % gfx->elements())[srcy * gfx->rowbytes()];
	for (int y = dsty; y <= dstendy; y++)
	{
		const uint8_t *srcp = &src_data[srcx];
		uint8_t *dstp = &dst.pix(y, dstx);
		for (int x = dstx; x <= dstendx; x++)
		{
			if (*srcp)
				*dstp = 1;
			srcp += dx;
			dstp++;
		}
		src_data += dy;
	}
}

void supracan_um6618_video_device::draw_sprite_tile_masked(bitmap_ind16 &dst, bitmap_ind8 &mask, bitmap_ind8 &priomap, const rectangle &cliprect, gfx_element *gfx, int tile,
	int palette, bool xflip, bool yflip, int dstx, int dsty, int prio)
{
	// compute final pixel in X and exit if we are entirely clipped
	int dstendx = dstx + 7;
	if (dstx > cliprect.right() || dstendx < cliprect.left())
		return;

	// apply left clip
	int srcx = 0;
	if (dstx < cliprect.left())
	{
		srcx = cliprect.left() - dstx;
		dstx = cliprect.left();
	}

	// apply right clip
	if (dstendx > cliprect.right())
		dstendx = cliprect.right();

	// compute final pixel in Y and exit if we are entirely clipped
	int dstendy = dsty + 7;
	if (dsty > cliprect.bottom() || dstendy < cliprect.top())
		return;

	// apply top clip
	int srcy = 0;
	if (dsty < cliprect.top())
	{
		srcy = cliprect.top() - dsty;
		dsty = cliprect.top();
	}

	// apply bottom clip
	if (dstendy > cliprect.bottom())
		dstendy = cliprect.bottom();

	// apply X flipping
	int dx = 1;
	if (xflip)
	{
		srcx = 7 - srcx;
		dx = -dx;
	}

	// apply Y flipping
	int dy = gfx->rowbytes();
	if (yflip)
	{
		srcy = 7 - srcy;
		dy = -dy;
	}

	const int color = gfx->colorbase() + gfx->granularity() * (palette % gfx->colors());
	const uint8_t *src_data = &gfx->get_data(tile % gfx->elements())[srcy * gfx->rowbytes()];
	for (int y = dsty; y <= dstendy; y++)
	{
		const uint8_t *srcp = &src_data[srcx];
		uint16_t *dstp = &dst.pix(y, dstx);
		uint8_t *priop = &priomap.pix(y, dstx);
		uint8_t *maskp = &mask.pix(y, dstx);
		for (int x = dstx; x <= dstendx; x++)
		{
			const uint32_t srcdata = *srcp;
			if (srcdata != 0 && *maskp != 0)
			{
				*dstp = (uint16_t)(srcdata + color);
				*priop = (*priop & 0xf0) | (uint8_t)prio;
			}
			srcp += dx;
			dstp++;
			priop++;
			maskp++;
		}
		src_data += dy;
	}
}

void supracan_um6618_video_device::draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &maskmap, bitmap_ind8 &priomap, const rectangle &cliprect)
{
	uint16_t *supracan_vram = m_vram;

//      [0]
//      -e-- ---- ---- ---- sprite enable?
//      ---h hhh- ---- ---- Y size (not always right)
//      ---- ---y yyyy yyyy Y position
//      [1]
//      bbbb ---- ---- ---- Tile bank
//      ---- h--- ---- ---- Horizontal flip
//      ---- -v-- ---- ---- Vertical flip
//      ---- --mm ---- ---- Masking mode
//      ---- ---- ---- -www X size
//      [2]
//      zzz- ---- ---- ---- X scale
//      ---- ???- ---- ---- Unknown, but often written.
//                          Values include 111 and 110 for the Super A'Can logo, 110 in the Sango Fighter intro, and 101/100 in the Boom Zoo intro.
//      ---- ---x xxxx xxxx X position
//      [3]
//      d--- ---- ---- ---- Direct Sprite (use details from here, not looked up in vram)
//      -ooo oooo oooo oooo Sprite address

	uint32_t skip_count = 0;
	uint32_t start_word = (m_sprite_base_addr >> 1) + skip_count * 4;
	uint32_t end_word = start_word + (m_sprite_count - skip_count) * 4;
	int region = (m_sprite_flags & 1) ? 0 : 1; // 8bpp : 4bpp

	static const uint16_t VRAM_MASK = 0xffff;

	for (int i = start_word; i < end_word; i += 4)
	{
		int x = supracan_vram[i + 2] & 0x01ff;
		int y = supracan_vram[i + 0] & 0x01ff;

		int sprite_offset = (supracan_vram[i + 3])<< 1;

		int bank = (supracan_vram[i + 1] & 0xf000) >> 12;
		int mask = (supracan_vram[i + 1] & 0x0300) >> 8;
		int sprite_xflip = (supracan_vram[i + 1] & 0x0800) >> 11;
		int sprite_yflip = (supracan_vram[i + 1] & 0x0400) >> 10;
		int prio = (supracan_vram[i + 2] >> 9) & 3;
		//int xscale = supracan_vram[i + 2] >> 13;
		gfx_element *gfx = m_gfxdecode->gfx(region);

		// wraparound
		if (y >= 0x180) y -= 0x200;
		if (x >= 0x180) x -= 0x200;

		if ((supracan_vram[i + 0] & 0x4000))
		{
		#if 0
			printf("%d (unk %02x) (enable %02x) (unk Y2 %02x, %02x) (y pos %02x) (bank %01x) (flip %01x) (unknown %02x) (x size %02x) (xscale %01x) (unk %01x) (xpos %02x) (code %04x)\n", i,
				(supracan_vram[i + 0] & 0x8000) >> 15,
				(supracan_vram[i + 0] & 0x4000) >> 14,
				(supracan_vram[i + 0] & 0x2000) >> 13,
				(supracan_vram[i + 0] & 0x1e00) >> 8,
				(supracan_vram[i + 0] & 0x01ff),
				(supracan_vram[i + 1] & 0xf000) >> 12,
				(supracan_vram[i + 1] & 0x0c00) >> 10,
				(supracan_vram[i + 1] & 0x03f0) >> 4,
				(supracan_vram[i + 1] & 0x000f),
				(supracan_vram[i + 2] & 0xf000) >> 12,
				(supracan_vram[i + 2] & 0x0e00) >> 8,
				(supracan_vram[i + 2] & 0x01ff) >> 0,
				(supracan_vram[i + 3] & 0xffff));
		#endif

			if (supracan_vram[i + 3] & 0x8000)
			{
				uint16_t data = supracan_vram[i + 3];
				int tile = (bank * 0x200) + (data & 0x03ff);

				int palette = (data & 0xf000) >> 12; // this might not be correct, due to the & 0x8000 condition above this would force all single tile sprites to be using palette >= 0x8 only

				// printf("sprite data %04x %04x %04x %04x\n", supracan_vram[i+0] , supracan_vram[i+1] , supracan_vram[i+2] ,supracan_vram[i+3]  );

				if (mask > 1)
					draw_sprite_tile_mask(maskmap, cliprect, gfx, tile, sprite_xflip, sprite_yflip, x, y);
				else if (mask == 1)
					draw_sprite_tile_masked(bitmap, maskmap, priomap, cliprect, gfx, tile, palette, sprite_xflip, sprite_yflip, x, y, prio);
				else
					draw_sprite_tile(bitmap, priomap, cliprect, gfx, tile, palette, sprite_xflip, sprite_yflip, x, y, prio);
			}
			else
			{
				int xsize = 1 << (supracan_vram[i + 1] & 7);
				int ysize = ((supracan_vram[i + 0] & 0x1e00) >> 9) + 1;

				// I think the xsize must influence the ysize somehow, there are too many conflicting cases otherwise
				// there don't appear to be any special markers in the actual looked up tile data to indicate skip / end of list

				for (int ytile = 0; ytile < ysize; ytile++)
				{
					for (int xtile = 0; xtile < xsize; xtile++)
					{
						uint16_t data = supracan_vram[(sprite_offset + ytile * xsize + xtile) & VRAM_MASK];
						int tile = (bank * 0x200) + (data & 0x03ff);
						int palette = (data & 0xf000) >> 12;

						int xpos = sprite_xflip ? (x - (xtile + 1) * 8 + xsize * 8) : (x + xtile * 8);
						int ypos = sprite_yflip ? (y - (ytile + 1) * 8 + ysize * 8) : (y + ytile * 8);

						int tile_xflip = sprite_xflip ^ ((data & 0x0800) >> 11);
						int tile_yflip = sprite_yflip ^ ((data & 0x0400) >> 10);

						if (mask > 1)
							draw_sprite_tile_mask(maskmap, cliprect, gfx, tile, tile_xflip, tile_yflip, xpos, ypos);
						else if (mask == 1)
							draw_sprite_tile_masked(bitmap, maskmap, priomap, cliprect, gfx, tile, palette, tile_xflip, tile_yflip, xpos, ypos, prio);
						else
							draw_sprite_tile(bitmap, priomap, cliprect, gfx, tile, palette, tile_xflip, tile_yflip, xpos, ypos, prio);
					}
				}
			}

#if 0
			if (xscale == 0) continue;
			uint32_t delta = (1 << 17) / xscale;
			for (int sy = 0; sy < ysize * 8; sy++)
			{
				uint16_t *src = &sprite_bitmap->pix(sy);
				uint16_t *dst = &bitmap.pix(y + sy);
				uint32_t dx = x << 16;
				for (int sx = 0; sx < xsize * 8; sx++)
				{
					dst[dx >> 16] = src[sx];
					dx += delta;
				}
			}
#endif

		}
	}
}



void supracan_um6618_video_device::mark_active_tilemap_all_dirty(int layer)
{
	int xsize = 0;
	int ysize = 0;

	int which_tilemap_size = get_tilemap_dimensions(xsize, ysize, layer);
//  for (int i=0;i<4;i++)
//    tilemap_mark_all_tiles_dirty(m_tilemap_sizes[layer][i]);
	m_tilemap_sizes[layer][which_tilemap_size]->mark_all_dirty();
}



/* draws tilemap with linescroll OR columnscroll to 16-bit indexed bitmap */
void supracan_um6618_video_device::draw_roz_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy, int wraparound/*, int columnscroll, uint32_t* scrollram*/, int transmask)
{
	bitmap_ind16 &srcbitmap = tmap->pixmap();
	const int xmask = srcbitmap.width() - 1;
	const int ymask = srcbitmap.height() - 1;
	const int widthshifted = srcbitmap.width() << 16;
	const int heightshifted = srcbitmap.height() << 16;

	/* pre-advance based on the cliprect */
	startx += cliprect.min_x * incxx + cliprect.min_y * incyx;
	starty += cliprect.min_x * incxy + cliprect.min_y * incyy;

	/* extract start/end points */
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	{
		/* loop over rows */
		while (sy <= ey)
		{
			/* initialize X counters */
			int x = sx;
			uint32_t cx = startx;
			uint32_t cy = starty;

			/* get dest and priority pointers */
			uint16_t *dest = &bitmap.pix(sy, sx);

			/* loop over columns */
			while (x <= ex)
			{
				if ((wraparound) || (cx < widthshifted && cy < heightshifted)) // not sure how this will cope with no wraparound, but row/col scroll..
				{
					#if 0
					if (columnscroll)
					{
						int scroll = 0; // scrollram[(cx>>16)&0x3ff]);

						uint16_t data = &srcbitmap.pix(((cy >> 16) - scroll) & ymask, (cx >> 16) & xmask)[0];

						if ((data & transmask) != 0)
							dest[0] = data;
					}
					else
					#endif
					{
						int scroll = 0;//scrollram[(cy>>16)&0x3ff]);
						uint16_t data =  srcbitmap.pix((cy >> 16) & ymask, ((cx >> 16) - scroll) & xmask);

						if ((data & transmask) != 0)
							*dest = data;
					}
				}

				/* advance in X */
				cx += incxx;
				cy += incxy;
				x++;
				dest++;
			}

			/* advance in Y */
			startx += incyx;
			starty += incyy;
			sy++;
		}
	}
}


// VIDEO FLAGS                  ROZ MODE            TILEMAP FLAGS
//
//  Bit                         Bit                 Bit
// 15-9: Unknown                15-13: Priority?    15-13: Priority?
//    8: X ht. (256/320)        12: Unknown         12: Unknown
//    7: Tilemap 0 enable       11-8: Dims          11-8: Dims
//    6: Tilemap 1 enable       7-6: Unknown        7-6: Unknown
//    5: Tilemap 2 enable?      5: Wrap             5: Wrap
//    3: Sprite enable          4-2: Unknown        4-2: Mosaic
//    2: ROZ enable             1-0: Bit Depth      1-0: Bit Depth
//  1-0: Unknown

//                      Video Flags                 ROZ Mode                    Tilemap 0   Tilemap 1   Tilemap 2   VF Unk0
// A'Can logo:          120e: 0001 0010 0000 1110   4020: 0100 0000 0010 0000   4620        ----        ----        0x09
// Boomzoo Intro:       9a82: 1001 1010 1000 0010   0402: 0000 0100 0000 0010   6400        6400        4400        0x4d
// Boomzoo Title:       9acc: 1001 1010 1100 1100   0402: 0000 0100 0000 0010   6400        6400        4400        0x4d
// C.U.G. Intro:        11c8: 0001 0001 1100 1000   0402: 0000 0100 0000 0010   2400        4400        6400        0x08
// C.U.G. Title:        11cc: 0001 0001 1100 1100   0602: 0000 0110 0000 0010   2600        4600        ----        0x08
// Speedy Dragon Logo:  0388: 0000 0011 1000 1000   4020: 0100 0000 0010 0000   6c20        6c20        2600        0x01
// Speedy Dragon Title: 038c: 0000 0011 1000 1100   2603: 0010 0110 0000 0011   6c20        2c20        2600        0x01
// Sango Fighter Intro: 03c8: 0000 0011 1100 1000   ----: ---- ---- ---- ----   6c20        4620        ----        0x01
// Sango Fighter Game:  03ce: 0000 0011 1100 1110   0622: 0000 0110 0010 0010   2620        4620        ----        0x01

uint32_t supracan_um6618_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// treat the sprites as frame-buffered and only update the buffer when drawing scanline 0 - this might not be true!

	if (0)
	{
		if (cliprect.min_y == 0x00)
		{
			const rectangle &visarea = screen.visible_area();

			m_sprite_final_bitmap.fill(0x00, visarea);
			m_sprite_mask_bitmap.fill(0x00, cliprect);
			m_prio_bitmap.fill(0xff, cliprect);
			bitmap.fill(0x80, visarea);

			draw_sprites(m_sprite_final_bitmap, m_sprite_mask_bitmap, m_prio_bitmap, visarea);
		}
	}
	else
	{
		m_sprite_final_bitmap.fill(0x00, cliprect);
		m_sprite_mask_bitmap.fill(0x00, cliprect);
		m_prio_bitmap.fill(0xff, cliprect);
		bitmap.fill(0x80, cliprect);

		draw_sprites(m_sprite_final_bitmap, m_sprite_mask_bitmap, m_prio_bitmap, cliprect);
	}

	// mix screen
	int xsize = 0, ysize = 0;
//  int tilemap_num;
	int priority = 0;

	for (int pri = 7; pri >= 0; pri--)
	{
		for (int layer = 3; layer >=0; layer--)
		{
		//  popmessage("%04x\n",m_video_flags);
			int enabled = 0;

			if (m_video_flags & 0x04)
				if (layer==3) enabled = 1;

			if (m_video_flags & 0x80)
				if (layer==0) enabled = 1;

			if (m_video_flags & 0x40)
				if (layer==1) enabled = 1;

			if (m_video_flags & 0x20)
				if (layer==2) enabled = 1;


			if (layer==3)
				priority = ((m_roz_mode >> 13) & 7); // roz case
			else
				priority = ((m_tilemap_flags[layer] >> 13) & 7); // normal cases


			if (priority == pri)
			{
//            tilemap_num = layer;
				int which_tilemap_size = get_tilemap_dimensions(xsize, ysize, layer);
				bitmap_ind16 &src_bitmap = m_tilemap_sizes[layer][which_tilemap_size]->pixmap();
				int gfx_region = get_tilemap_region(layer);
				int transmask = 0xff;

				switch (gfx_region)
				{
					case 0: transmask = 0xff; break;
					case 1: transmask = 0x0f; break;
					case 2: transmask = 0x03; break;
					case 3: transmask = 0x01; break;
					case 4: transmask = 0x01; break;
				}

				if (enabled)
				{
					if (layer != 3) // standard layers, NOT roz
					{
						int wrap = (m_tilemap_flags[layer] & 0x20);

						int scrollx = m_tilemap_scrollx[layer];
						int scrolly = m_tilemap_scrolly[layer];

						if (scrollx & 0x8000) scrollx -= 0x10000;
						if (scrolly & 0x8000) scrolly -= 0x10000;

						int mosaic_count = (m_tilemap_flags[layer] & 0x001c) >> 2;
						int mosaic_mask = 0xffffffff << mosaic_count;

						// yes, it will draw a single line if you specify a cliprect as such (partial updates...)

						for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
						{
							// these will have to change to uint32_t* etc. once alpha blending is supported
							uint16_t* screen = &bitmap.pix(y);

							int actualy = y & mosaic_mask;
							int realy = actualy + scrolly;

							if (!wrap)
								if (scrolly + y < 0 || scrolly + y > ((ysize * 8) - 1))
									continue;

							uint16_t* src = &src_bitmap.pix(realy & ((ysize * 8) - 1));
							uint8_t* priop = &m_prio_bitmap.pix(y);

							for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
							{
								int actualx = x & mosaic_mask;
								int realx = actualx + scrollx;

								if (!wrap)
									if (scrollx + x < 0 || scrollx + x > ((xsize * 8) - 1))
										continue;

								uint16_t srcpix = src[realx & ((xsize * 8) - 1)];

								if ((srcpix & transmask) != 0 && priority < (priop[x] >> 4))
								{
									screen[x] = srcpix;
									priop[x] = (priop[x] & 0x0f) | (priority << 4);
								}
							}
						}
					}
					else
					{
						int wrap = m_roz_mode & 0x20;

						int incxx = m_roz_coeffa;
						int incyy = m_roz_coeffd;

						int incxy = m_roz_coeffc;
						int incyx = m_roz_coeffb;

						int scrollx = m_roz_scrollx;
						int scrolly = m_roz_scrolly;

						if (incyx & 0x8000) incyx -= 0x10000;
						if (incxy & 0x8000) incxy -= 0x10000;

						if (incyy & 0x8000) incyy -= 0x10000;
						if (incxx & 0x8000) incxx -= 0x10000;

						//popmessage("%04x %04x\n",m_video_flags, m_roz_mode);

						// roz mode..
						//4020 = enabled speedyd
						//6c22 = enabled speedyd
						//2c22 = enabled speedyd
						//4622 = disabled jttlaugh
						//2602 = disabled monopoly
						//0402 = disabled (sango title)
						// or is it always enabled, and only corrupt because we don't clear ram properly?
						// (probably not this register?)

						if (!(m_roz_mode & 0x0200) && (m_roz_mode & 0xf000)) // HACK - Not trusted: Acan Logo, Speedy Dragon Intro, Speed Dragon Bonus stage need it.  Monopoly and JTT *don't* causes graphical issues
						{
							// NOT accurate, causes issues when the attract mode loops and the logo is shown the 2nd time in some games - investigate
							for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
							{
								rectangle clip(cliprect.min_x, cliprect.max_x, y, y);

								scrollx = (m_roz_scrollx);
								scrolly = (m_roz_scrolly);
								incxx = (m_roz_coeffa);

								incxx += m_vram[m_roz_unk_base0/2 + y];

								scrollx += m_vram[m_roz_unk_base1/2 + y * 2] << 16;
								scrollx += m_vram[m_roz_unk_base1/2 + y * 2 + 1];

								scrolly += m_vram[m_roz_unk_base2/2 + y * 2] << 16;
								scrolly += m_vram[m_roz_unk_base2/2 + y * 2 + 1];

								if (incxx & 0x8000) incxx -= 0x10000;

								if (m_vram[m_roz_unk_base0/2 + y]) // incxx = 0, no draw?
									draw_roz_layer(bitmap, clip, m_tilemap_sizes[layer][which_tilemap_size], scrollx<<8, scrolly<<8, incxx<<8, incxy<<8, incyx<<8, incyy<<8, wrap, transmask);
							}
						}
						else
						{
							draw_roz_layer(bitmap, cliprect, m_tilemap_sizes[layer][which_tilemap_size], scrollx<<8, scrolly<<8, incxx<<8, incxy<<8, incyx<<8, incyy<<8, wrap, transmask);
						}
					}
				}
			}
		}
	}


	// combine sprites
	if (m_video_flags & 0x08)
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			uint16_t* dstp = &bitmap.pix(y);
			uint8_t* priop = &m_prio_bitmap.pix(y);
			uint16_t* spritep = &m_sprite_final_bitmap.pix(y);

			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				uint16_t sprite_pix = spritep[x];
				uint8_t tile_prio = priop[x] >> 4;
				uint8_t sprite_prio = priop[x] & 0x0f;
				if (sprite_pix != 0 && sprite_prio <= tile_prio)
				{
					dstp[x] = sprite_pix;
				}
			}
		}
	}

	return 0;
}



// swap address around so that 64x64 tile can be decoded as 8x8 tiles..
void supracan_um6618_video_device::write_swapped_byte(int offset, uint8_t byte)
{
	int swapped_offset = bitswap<32>(offset, 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,2,1,0,6,5,4,3);

	m_vram_addr_swapped[swapped_offset] = byte;
}

void supracan_um6618_video_device::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);

	// hack for 1bpp layer at startup
	write_swapped_byte(offset * 2, data >> 8);
	write_swapped_byte(offset * 2 + 1, (data & 0x00ff));

	// mark tiles of each depth as dirty
	m_gfxdecode->gfx(0)->mark_dirty((offset * 2) / 64);
	m_gfxdecode->gfx(1)->mark_dirty((offset * 2) / 32);
	m_gfxdecode->gfx(2)->mark_dirty((offset * 2) / 16);
	m_gfxdecode->gfx(3)->mark_dirty((offset * 2) / 512);
	m_gfxdecode->gfx(4)->mark_dirty((offset * 2) / 8);

}


void supracan_um6618_video_device::video_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//address_space &mem = m_maincpu->space(AS_PROGRAM);

	// if any of this changes we need a partial update (see sango fighters intro)
	m_screen->update_partial(m_screen->vpos());

	COMBINE_DATA(&m_video_regs[offset]);
	data = m_video_regs[offset];

	switch (offset)
	{
	case 0x10/2: // Byte count
		LOGMASKED(LOG_SPRDMA, "sprite dma word count: %04x\n", data);
		m_sprdma_regs.count = data;
		break;
	case 0x12/2: // Destination address MSW
		m_sprdma_regs.dst &= 0x0000ffff;
		m_sprdma_regs.dst |= data << 16;
		LOGMASKED(LOG_SPRDMA, "sprite dma dest msw: %04x\n", data);
		break;
	case 0x14/2: // Destination address LSW
		m_sprdma_regs.dst &= 0xffff0000;
		m_sprdma_regs.dst |= data;
		LOGMASKED(LOG_SPRDMA, "sprite dma dest lsw: %04x\n", data);
		break;
	case 0x16/2: // Source word increment
		LOGMASKED(LOG_SPRDMA, "sprite dma dest word inc: %04x\n", data);
		m_sprdma_regs.dst_inc = data;
		break;
	case 0x18/2: // Source address MSW
		m_sprdma_regs.src &= 0x0000ffff;
		m_sprdma_regs.src |= data << 16;
		LOGMASKED(LOG_SPRDMA, "sprite dma src msw: %04x\n", data);
		break;
	case 0x1a/2: // Source address LSW
		LOGMASKED(LOG_SPRDMA, "sprite dma src lsw: %04x\n", data);
		m_sprdma_regs.src &= 0xffff0000;
		m_sprdma_regs.src |= data;
		break;
	case 0x1c/2: // Source word increment
		LOGMASKED(LOG_SPRDMA, "sprite dma src word inc: %04x\n", data);
		m_sprdma_regs.src_inc = data;
		break;
	case 0x1e/2:
		LOGMASKED(LOG_SPRDMA, "video_w: Kicking off a DMA from %08x to %08x, %d bytes (%04x)\n", m_sprdma_regs.src, m_sprdma_regs.dst, m_sprdma_regs.count, data);

		/* TODO: what's 0x2000 and 0x4000 for? */
		if (data & 0x8000)
		{
			if (data & 0x2000 || data & 0x4000)
			{
				m_sprdma_regs.dst |= 0xf40000;
			}

			if (data & 0x2000)
			{
				//m_sprdma_regs.count <<= 1;
			}

			for (int i = 0; i <= m_sprdma_regs.count; i++)
			{
				if (data & 0x0100) // dma 0x00 fill (or fixed value?)
				{
					m_write_maincpu_space(m_sprdma_regs.dst, 0x0000);

					m_sprdma_regs.dst += 2 * m_sprdma_regs.dst_inc;
				}
				else
				{
					m_write_maincpu_space(m_sprdma_regs.dst, m_read_maincpu_space(m_sprdma_regs.src));

					m_sprdma_regs.dst += 2 * m_sprdma_regs.dst_inc;
					m_sprdma_regs.src += 2 * m_sprdma_regs.src_inc;
				}
			}
		}
		else
		{
			LOGMASKED(LOG_SPRDMA | LOG_UNKNOWNS, "video_w: Attempting to kick off a DMA without bit 15 set! (%04x)\n", data);
		}
		break;
	case 0x08/2:
		if (data != m_video_flags)
		{
			LOGMASKED(LOG_VIDEO, "video_flags = %04x\n", data);

			m_video_flags = data;

			rectangle visarea = m_screen->visible_area();

			visarea.set(0, ((m_video_flags & 0x100) ? 320 : 256) - 1, 8, 232 - 1);
			m_screen->configure(348, 256, visarea, m_screen->frame_period().attoseconds());
		}
		break;
	case 0x0a/2:
		// raster interrupt
		LOGMASKED(LOG_IRQS, "Raster 'line on' IRQ Trigger write? = %04x\n", data);
		if (data & 0x8000)
		{
			m_line_on_timer->adjust(m_screen->time_until_pos((data & 0x00ff), 0));
		}
		else
		{
			m_line_on_timer->adjust(attotime::never);
		}
		break;

	case 0x0c/2:
		LOGMASKED(LOG_IRQS, "Raster 'line off' IRQ Trigger write? = %04x\n", data);
		if (data & 0x8000)
		{
			m_line_off_timer->adjust(m_screen->time_until_pos(data & 0x00ff, 0));
		}
		else
		{
			m_line_off_timer->adjust(attotime::never);
		}
		break;

	/* Sprites */
	case 0x20/2: m_sprite_base_addr = data << 2; LOGMASKED(LOG_SPRITES, "sprite_base_addr = %04x\n", data); break;
	case 0x22/2: m_sprite_count = data + 1; LOGMASKED(LOG_SPRITES, "sprite_count = %d\n", data + 1); break;
	case 0x26/2: m_sprite_flags = data; LOGMASKED(LOG_SPRITES, "sprite_flags = %04x\n", data); break;

	/* Tilemap 0 */
	case 0x100/2: m_tilemap_flags[0] = data; LOGMASKED(LOG_TILEMAP0, "tilemap_flags[0] = %04x\n", data); break;
	case 0x104/2: m_tilemap_scrollx[0] = data; LOGMASKED(LOG_TILEMAP0, "tilemap_scrollx[0] = %04x\n", data); break;
	case 0x106/2: m_tilemap_scrolly[0] = data; LOGMASKED(LOG_TILEMAP0, "tilemap_scrolly[0] = %04x\n", data); break;
	case 0x108/2: m_tilemap_base_addr[0] = data << 1; LOGMASKED(LOG_TILEMAP0, "tilemap_base_addr[0] = %05x\n", data << 2); break;
	case 0x10a/2: m_tilemap_mode[0] = data; LOGMASKED(LOG_TILEMAP0, "tilemap_mode[0] = %04x\n", data); break;
	/*
	       fedc ba98 7654 3210

	 0x00: .pp. PPPF CXSM MMfF

		. = reserved
		p = PRT              Priority 
		P = PS               Page Size
		F = FS               Tile Size, 0 = 8x8, 1 = 16x16
		C = C2               2 Colour Mode
		X = MIX              Depth? Blend? 0 = 16, 1 = 256
		S = SC               Wrap
		M = MSC              Mosaic
		f = XF               X Flip / Mirror
		F = YF               Y Flip / Mirror

	 0x02: .XY. SMIA .... ....

		. = reserved
		X = XO EN            X Offset Enable
		Y = YO EN            Y Offset Enable
		S = YM SEL
		M = YM EN            Y Mapping Enable
		I = IM               Alt format for tile data?
		A = A1               Linescroll?

	 0x04: .... .XXX XXXX XXXX  
	 0x06: .... .YYY YYYY YYYY

		. = reserved
		X = SPX             Scroll Position X
		Y = SPY             Scroll Position Y

	 0x08: PPPP PPPP PPPP P...    

		P = PNTBK 17-5      Pointer to Tilemap data

	 0x0A: GGGG G... .... ....

		G = PGTBK 17-13     Pointer to Graphic data

	 0x0C: XXXX XXXX XXXX XXXX
	 0x0E: YYYY YYYY YYYY YYYY

		X = XOBK 17-2       Pointer to X Offset data?
		Y = YMBK 17-2       Pointer to rowscroll data

	 0x10: YYYY YYYY YYY. ....

	    Y = YOBK 17-7       Pointer to Y Offset data?

	*/

	/* Tilemap 1 */
	case 0x120/2: m_tilemap_flags[1] = data; LOGMASKED(LOG_TILEMAP1, "tilemap_flags[1] = %04x\n", data); break;
	case 0x124/2: m_tilemap_scrollx[1] = data; LOGMASKED(LOG_TILEMAP1, "tilemap_scrollx[1] = %04x\n", data); break;
	case 0x126/2: m_tilemap_scrolly[1] = data; LOGMASKED(LOG_TILEMAP1, "tilemap_scrolly[1] = %04x\n", data); break;
	case 0x128/2: m_tilemap_base_addr[1] = data << 1; LOGMASKED(LOG_TILEMAP1, "tilemap_base_addr[1] = %05x\n", data << 2); break;
	case 0x12a/2: m_tilemap_mode[1] = data; LOGMASKED(LOG_TILEMAP1, "tilemap_mode[1] = %04x\n", data); break;

	/* Tilemap 2? */
	case 0x140/2: m_tilemap_flags[2] = data; LOGMASKED(LOG_TILEMAP2, "tilemap_flags[2] = %04x\n", data); break;
	case 0x144/2: m_tilemap_scrollx[2] = data; LOGMASKED(LOG_TILEMAP2, "tilemap_scrollx[2] = %04x\n", data); break;
	case 0x146/2: m_tilemap_scrolly[2] = data; LOGMASKED(LOG_TILEMAP2, "tilemap_scrolly[2] = %04x\n", data); break;
	case 0x148/2: m_tilemap_base_addr[2] = data << 1; LOGMASKED(LOG_TILEMAP2, "tilemap_base_addr[2] = %05x\n", data << 2); break;
	case 0x14a/2: m_tilemap_mode[2] = data; LOGMASKED(LOG_TILEMAP2, "tilemap_mode[2] = %04x\n", data); break;

	/* ROZ */
	case 0x180/2: m_roz_mode = data; LOGMASKED(LOG_ROZ, "roz_mode = %04x\n", data); break;
	case 0x184/2: m_roz_scrollx = (data << 16) | (m_roz_scrollx & 0xffff); m_roz_changed |= 1; LOGMASKED(LOG_ROZ, "roz_scrollx = %08x\n", m_roz_scrollx); break;
	case 0x186/2: m_roz_scrollx = (data) | (m_roz_scrollx & 0xffff0000); m_roz_changed |= 1; LOGMASKED(LOG_ROZ, "roz_scrollx = %08x\n", m_roz_scrollx); break;
	case 0x188/2: m_roz_scrolly = (data << 16) | (m_roz_scrolly & 0xffff); m_roz_changed |= 2; LOGMASKED(LOG_ROZ, "roz_scrolly = %08x\n", m_roz_scrolly); break;
	case 0x18a/2: m_roz_scrolly = (data) | (m_roz_scrolly & 0xffff0000); m_roz_changed |= 2; LOGMASKED(LOG_ROZ, "roz_scrolly = %08x\n", m_roz_scrolly); break;
	case 0x18c/2: m_roz_coeffa = data; LOGMASKED(LOG_ROZ, "roz_coeffa = %04x\n", data); break;
	case 0x18e/2: m_roz_coeffb = data; LOGMASKED(LOG_ROZ, "roz_coeffb = %04x\n", data); break;
	case 0x190/2: m_roz_coeffc = data; LOGMASKED(LOG_ROZ, "roz_coeffc = %04x\n", data); break;
	case 0x192/2: m_roz_coeffd = data; LOGMASKED(LOG_ROZ, "roz_coeffd = %04x\n", data); break;
	case 0x194/2: m_roz_base_addr = data << 1; LOGMASKED(LOG_ROZ, "roz_base_addr = %05x\n", data << 2); break;
	case 0x196/2: m_roz_tile_bank = data; LOGMASKED(LOG_ROZ, "roz_tile_bank = %04x\n", data); break; //tile bank
	case 0x198/2: m_roz_unk_base0 = data << 2; LOGMASKED(LOG_ROZ, "roz_unk_base0 = %05x\n", data << 2); break;
	case 0x19a/2: m_roz_unk_base1 = data << 2; LOGMASKED(LOG_ROZ, "roz_unk_base1 = %05x\n", data << 2); break;
	case 0x19e/2: m_roz_unk_base2 = data << 2; LOGMASKED(LOG_ROZ, "roz_unk_base2 = %05x\n", data << 2); break;

	case 0x1d0/2: m_unk_1d0 = data; LOGMASKED(LOG_UNKNOWNS, "unk_1d0 = %04x\n", data); break;

	case 0x1f0/2: // FIXME: this register is mostly not understood
		m_screen_mode_control = data;//(data & 8) ? 0 : 1;

		/*
		
		fedcba9876543210

		...........pplll

		. = reserved
		p = pmode
		l = num layer/depth config	

		l = 000 : 4/2, 4/2, 4/2, 4/2
		    001 : 16, 16, 4/2
			010 : 256, 16
			011 : 16/Y, 16/Y, 2
			100 : 256/Y, 4/2/Y
			101 : invalid
			110 : invalid
			111 : invalid

		p = 00 : tile mode
		    01 : bitmap mode
			10 : unknown
			11 : invalid

		*/
#if 0
		if (!m_screen_mode_control && !m_hbl_mask)
		{
			m_vblank_irq(0);
		}
#endif
		LOGMASKED(LOG_IRQS, "irq_mask = %04x\n", data);
		break;
	default:
		LOGMASKED(LOG_UNKNOWNS, "video_w: Unknown register: %08x = %04x & %04x\n", 0xf00000 + (offset << 1), data, mem_mask);
		break;
	}
//  m_video_regs[offset] = data;
}


uint16_t supracan_um6618_video_device::video_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_video_regs[offset];

	switch (offset)
	{
	case 0x00/2: // Video IRQ flags
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_HFVIDEO, "read video IRQ flags (%04x)\n", data);
			m_vblank_irq(0);
		}
		break;
	case 0x02/2: // Current scanline
		LOGMASKED(LOG_VIDEO, "read current scanline (%04x)\n", data);
		break;
	case 0x08/2: // Unknown (not video flags!) - gambling lord disagrees, it MUST read back what it wrote because it reads it before turning on/off layers and writes it back
		LOGMASKED(LOG_VIDEO, "read unkown 0x08 (%04x)\n", data);
		break;
	case 0x100/2:
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_TILEMAP0, "read tilemap_flags[0] (%04x)\n", data);
		}
		break;
	case 0x106/2:
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_TILEMAP0, "read tilemap_scrolly[0] (%04x)\n", data);
		}
		break;
	case 0x120/2:
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_TILEMAP1, "read tilemap_flags[1] (%04x)\n", data);
		}
		break;
	default:
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_UNKNOWNS, "video_r: Unknown register: %08x (%04x & %04x)\n", 0xf00000 + (offset << 1), data, mem_mask);
		}
		break;
	}

	return data;
}

TIMER_CALLBACK_MEMBER(supracan_um6618_video_device::hbl_callback)
{
	m_hblank_irq(1);

	m_hbl_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(supracan_um6618_video_device::line_on_callback)
{
	m_line_irq(1);

	m_line_on_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(supracan_um6618_video_device::line_off_callback)
{
	m_line_irq(0);

	m_line_on_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(supracan_um6618_video_device::video_callback)
{
	int vpos = m_screen->vpos();

	m_video_regs[0] &= ~0x0002;

	switch (vpos)
	{
	case 0:
		m_video_regs[0] &= 0x7fff;

		// we really need better management of this
		mark_active_tilemap_all_dirty(0);
		mark_active_tilemap_all_dirty(1);
		mark_active_tilemap_all_dirty(2);
		mark_active_tilemap_all_dirty(3);
		break;

	case 224: // FIXME: Son of Evil is pretty picky about this one, a timing of 240 makes it crash
		m_video_regs[0] |= 0x8000;
		break;

	case 240:
		if (m_screen_mode_control & 1)
		{
			LOGMASKED(LOG_IRQS, "Triggering VBL IRQ\n\n");
			m_vblank_irq(1);
		}
		break;
	}

	m_video_regs[1] = m_screen->vpos() - 16; // for son of evil, wants vblank active around 224 instead...

	m_hbl_timer->adjust(m_screen->time_until_pos(vpos, 320));
	m_video_timer->adjust(m_screen->time_until_pos((vpos + 1) % 256, 0));
}



int supracan_um6618_video_device::get_tilemap_region(int layer)
{
	// HACK!!!
	if (layer == 2)
	{
		return 2;
	}

	if (layer == 3)
	{
		// roz layer
		static const int s_roz_mode_lut[4] = { 4, 2, 1, 0 };
		return s_roz_mode_lut[m_roz_mode & 3];
	}
	else
	{
		// normal layers
		if ((m_tilemap_mode[layer] & 0x7000) == 0x7000)
		{
			return 2;
		}
		return 1;
	}

}

void supracan_um6618_video_device::get_tilemap_info_common(int layer, tile_data &tileinfo, int count)
{
	uint16_t* supracan_vram = m_vram;

	uint32_t base = m_tilemap_base_addr[layer];
	int gfx_mode = (m_tilemap_mode[layer] & 0x7000) >> 12;
	int region = get_tilemap_region(layer);

	count += base;

	uint16_t tile_bank = 0;
	uint16_t palette_bank = 0;
	switch (gfx_mode)
	{
	case 7:
		tile_bank = 0x1c00;
		palette_bank = 0x00;
		break;

	case 6: // gambling lord
		tile_bank = 0x0c00;
		palette_bank = 0x00;
		break;

	case 4:
		tile_bank = 0x800;
		palette_bank = 0x00;
		break;

	case 2:
		tile_bank = 0x400;
		palette_bank = 0x00;
		break;

	case 0:
		tile_bank = 0;
		palette_bank = 0x00;
		break;

	default:
		//LOGMASKED(LOG_UNKNOWNS, "Unsupported tilemap mode: %d\n", (m_tilemap_mode[layer] & 0x7000) >> 12);
		break;
	}


	if (layer == 2)
	{
		tile_bank = 0x1000;
	}

	int tile = (supracan_vram[count] & 0x03ff) + tile_bank;
	int flipxy = (supracan_vram[count] & 0x0c00) >> 10;
	int palette = ((supracan_vram[count] & 0xf000) >> 12) + palette_bank;

	tileinfo.set(region, tile, palette, TILE_FLIPXY(flipxy));
}

void supracan_um6618_video_device::get_roz_tilemap_info(int layer, tile_data &tileinfo, int count)
{
	uint16_t* supracan_vram = m_vram;

	uint32_t base = m_roz_base_addr;

	int region = 1;
	uint16_t tile_bank = 0;
	uint16_t palette_bank = 0;

	region = get_tilemap_region(layer);

	switch (m_roz_mode & 3) // FIXME: fix gfx bpp order
	{
	case 0:
	{
		// HACK: case for startup logo
		// This isn't understood properly, it's rendering a single 64x64 tile, which for convenience we've rearranged and decoded as 8x8 for the tilemaps
		int tile = 0x880 + ((count & 7) * 2);
		// tile += (count & 0x070) >> 2;

		if (count & 0x20) tile ^= 1;
		tile |= (count & 0xc0) >> 2;

		tileinfo.set(region, tile, 0, 0);
		return;
	}

	case 1:
		tile_bank = (m_roz_tile_bank & 0xf000) >> 3;
		break;

	case 2:
		tile_bank = (m_roz_tile_bank & 0xf000) >> 3;
		break;

	case 3:
		tile_bank = (m_roz_tile_bank & 0xf000) >> 3;
		break;
	}

	count += base;

	int tile = (supracan_vram[count] & 0x03ff) + tile_bank;
	int flipxy = (supracan_vram[count] & 0x0c00) >> 10;
	int palette = ((supracan_vram[count] & 0xf000) >> 12) + palette_bank;

	tileinfo.set(region, tile, palette, TILE_FLIPXY(flipxy));
}



TILE_GET_INFO_MEMBER(supracan_um6618_video_device::get_tilemap0_tile_info)
{
	get_tilemap_info_common(0, tileinfo, tile_index);
}

TILE_GET_INFO_MEMBER(supracan_um6618_video_device::get_tilemap1_tile_info)
{
	get_tilemap_info_common(1, tileinfo, tile_index);
}

TILE_GET_INFO_MEMBER(supracan_um6618_video_device::get_tilemap2_tile_info)
{
	get_tilemap_info_common(2, tileinfo, tile_index);
}

TILE_GET_INFO_MEMBER(supracan_um6618_video_device::get_roz_tile_info)
{
	get_roz_tilemap_info(3, tileinfo, tile_index);
}




/* gfxdecode is retained for reference purposes but not otherwise used by the driver */
static const gfx_layout supracan_gfx8bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ STEP8(0, 8*8) },
	8*8*8
};



static const gfx_layout supracan_gfx4bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout supracan_gfx2bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	2,
	{ 0, 1 },
	{ 0*2, 1*2, 2*2, 3*2, 4*2, 5*2, 6*2, 7*2 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const uint32_t xtexlayout_xoffset[64] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
												24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
												45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 };

static const uint32_t xtexlayout_yoffset[64] = {  0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,
												9*64,10*64,11*64,12*64,13*64,14*64,15*64,
												16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
												24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64,
												32*64,33*64,34*64,35*64,36*64,37*64,38*64,39*64,
												40*64,41*64,42*64,43*64,44*64,45*64,46*64,47*64,
												48*64,49*64,50*64,51*64,52*64,53*64,54*64,55*64,
												56*64,57*64,58*64,59*64,60*64,61*64,62*64,63*64 };
static const gfx_layout supracan_gfx1bpp =
{
	64, 64,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	64*64,
	xtexlayout_xoffset,
	xtexlayout_yoffset
};

static const gfx_layout supracan_gfx1bpp_alt =
{
	8, 8,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_supracan )
	GFXDECODE_RAM( "vram", 0, supracan_gfx8bpp, 0, 1 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx4bpp, 0, 0x10 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx2bpp, 0, 0x40 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx1bpp, 0, 0x80 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx1bpp_alt, 0, 0x80 )
GFXDECODE_END


void supracan_um6618_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, "palette", FUNC(supracan_um6618_video_device::palette_init)).set_format(palette_device::xBGR_555, 32768);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_supracan);
}
