// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
/*****************************************************************************

    SunPlus GCM394-series SoC peripheral emulation (Video)

    This is very similar to spg2xx but with additional features, layers and modes

**********************************************************************/

/* lots of games, including wrlshunt are not copying tilemap data properly
   the analysis below is for wrlshunt, although gormiti could prove to be an easier case to look at
   while jak_ths and jak_swc might be more difficult (the latter uses line/bitmap mode, but still
   fails to copy the line data)


   --

   wrlshunt BG Tilemap location note

   background tilemap appears to be at 24ad30 - 24af87 (byte address) in RAM  == 125698 - 1257c3 (word address)
   there are pointers to this
   (2879-287a) = 98 56 12 00 (00125698) (main background tilemap data is at this address)
   (287b-287c) = 30 5e 12 00 (00125e30) (address for other layer tilemap) (or 'end' of above)
   where do we get these copied to registers or used as a source to copy from?


   -- callled from here
   058F79: call 054e56 (with values above, for tilemap 0)
   and
   058FB1: call 054e56 (for tilemap 1)
   (both of these are at the start of the function at 058F46, which we loop in at the moment, possible main loop for the menu?)

   there are other calls in the code, but those are the ones before sprites are uploaded for the menu

   --
    054E91: r4 = [bp+27] (contains lower part of address)
    054E92: ds:[r1++] = r4    -- write 5698  to 2879
    054E93: r4 = [bp+28] (contains upper part of address)
    054E94: ds:[r1] = r4   -- write 0012  to 287a

    (this is a huge function that ends at 55968, also has lots of calls in it)

    ---

    the base for tilemap params being written to RAM is 2879 + 0xe * tilmap number (0,1,2,3)
    the code to calculate this offset from base uses 32-bit multiplication and even sign extends the tilemap number before using it, making it
    look more complex than it really is!

    054E7B: 0B0D 0088 bp = bp + 0088
    054E7D: 9800      r4 = [bp+00]  -- which tilemap? (0,1,2,3)
    054E7E: 2B0D 0088 bp = bp - 0088

    054E80: 973C      r3 = r4 asr 4  -- sign extend tilemap 16-bit register r4 with r3 forming the upper word (always 0)
    054E81: 973B      r3 = r3 asr 4
    054E82: 973B      r3 = r3 asr 4
    054E83: 973B      r3 = r3 asr 4

    054E84: D688      push r3, r3 to [sp] -- push onto stack for use in call below
    054E85: D888      push r4, r4 to [sp]

    054E86: 964E      r3 = 0e -- store 0000 000e as the 32-bit value to multply with
    054E87: 9840      r4 = 00
    054E88: D890      push r3, r4 to [sp] -- push that onto stack for function call below

    054E89: F045 D706 call 05d706   -- returns result in r1,r2

    the result of this is then added to the base value of 2879 (which was stored earlier)
    an additional offset is then added for each parameter.

    this code is repeated multiple times, with slight changes

    ---

    by the time you hit 055098 (which is a switch on tilemap type to disable a tilemap) the following params have been put at
    2879 ( tilemap 0 call )
    2879 + 0x0e (tilemap 1 call )

    tmap0 params
    5698 0012 | 5E30 0012 | 0280 01E0   | 0002 0020 0020 0000 0000 0100 0000 0000
    125698    | 125e30    | = 640 = 480

    tmap1 params
    7280 000D | 89F0 000D | 0280 01E0   | 0002 0020 0020 0002 0000 0040 0000 0000
    0d7280    | 0d89f0    | = 640 = 480 |

    these parameter lists are not read after this? is there some kind of indirect dma mode, or is code not being called that should use them.
    plenty more code is called, including more that looks a lot like the above, some use of 707f and at the end of the funciton, code to
    write various tilemap registers, including reenabling the tilemap that was disabled around 055098.

    --


   if you return rand() on 707f reads sometimes you see
   [:maincpu] pc:053775: r4 = r4 lsr r3  (5698 0009) : [:maincpu] result 002b  (possible unrelated)

   (bg tile addressing is also done by tile #, like the sprites, not fixed step like smartfp)


*/


#include "emu.h"
#include "generalplus_gpl16250soc_video.h"

DEFINE_DEVICE_TYPE(GCM394_VIDEO, gcm394_video_device, "gcm394_video", "GeneralPlus GPL16250 System-on-a-Chip (Video)")

#define LOG_GCM394_VIDEO_PALETTE  (1U << 5)
#define LOG_GCM394_VIDEO_DMA      (1U << 4)
#define LOG_GCM394_TMAP_EXTRA     (1U << 3)
#define LOG_GCM394_TMAP           (1U << 2)
#define LOG_GCM394_VIDEO          (1U << 1)

#define VERBOSE             (LOG_GCM394_TMAP | LOG_GCM394_VIDEO_DMA | LOG_GCM394_VIDEO |LOG_GCM394_VIDEO_PALETTE)

#include "logmacro.h"


gcm394_base_video_device::gcm394_base_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	//device_gfx_interface(mconfig, *this, nullptr),
	device_video_interface(mconfig, *this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
//  m_scrollram(*this, "scrollram"),
	m_video_irq_cb(*this),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_space_read_cb(*this),
	m_rowscroll(*this, "^rowscroll"),
	m_rowzoom(*this, "^rowzoom"),
	m_pal_displaybank_high(0),
	m_pal_sprites(0x500),
	m_pal_back(0x100),
	m_alt_tile_addressing(0)
{
}

gcm394_video_device::gcm394_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gcm394_base_video_device(mconfig, GCM394_VIDEO, tag, owner, clock)
{
}

void gcm394_base_video_device::decodegfx(const char* tag)
{
	if (!memregion(tag))
		return;

	uint8_t* gfxregion = memregion(tag)->base();
	int gfxregionsize = memregion(tag)->bytes();

	if (1)
	{
		gfx_layout obj_layout =
		{
			16,16,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP16(0,4) },
			{ STEP16(0,4 * 16) },
			16 * 16 * 4
		};
		obj_layout.total = gfxregionsize / (16 * 16 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,16,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP32(0,4) },
			{ STEP16(0,4 * 32) },
			16 * 32 * 4
		};
		obj_layout.total = gfxregionsize / (16 * 32 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			16,32,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP16(0,4) },
			{ STEP32(0,4 * 16) },
			32 * 16 * 4
		};
		obj_layout.total = gfxregionsize / (32 * 16 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,32,
			0,
			4,
			{ STEP4(0,1) },
			{ STEP32(0,4) },
			{ STEP32(0,4 * 32) },
			32 * 32 * 4
		};
		obj_layout.total = gfxregionsize / (32 * 32 * 4 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			8,16,
			0,
			2,
			{ 0,1 },
			{ STEP8(0,2) },
			{ STEP16(0,2 * 8) },
			8 * 16 * 2
		};
		obj_layout.total = gfxregionsize / (8 * 16 * 2 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x40 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		const uint32_t texlayout_xoffset[64] = { STEP64(0,2) };
		const uint32_t texlayout_yoffset[32] = { STEP32(0,2 * 64) };

		gfx_layout obj_layout =
		{
			64,32,
			0,
			2,
			{ 0,1 },
			EXTENDED_XOFFS,
			EXTENDED_YOFFS,
			32 * 64 * 2,
			texlayout_xoffset,
			texlayout_yoffset
		};
		obj_layout.total = gfxregionsize / (16 * 32 * 2 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x40 * 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,32,
			0,
			8,
			{ STEP8(0,1) },
			{ STEP32(0,8) },
			{ STEP32(0,8 * 32) },
			32 * 32 * 8
		};
		obj_layout.total = gfxregionsize / (32 * 32 * 8 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x10, 0));
		m_maxgfxelement++;
	}

	if (1)
	{
		gfx_layout obj_layout =
		{
			32,32,
			0,
			6,
			{ 0,1,2,3,4,5 },
			{ STEP32(0,6) },
			{ STEP32(0,6 * 32) },
			32 * 32 * 6
		};
		obj_layout.total = gfxregionsize / (32 * 32 * 6 / 8);
		m_gfxdecode->set_gfx(m_maxgfxelement, std::make_unique<gfx_element>(m_palette, obj_layout, gfxregion, 0, 0x40, 0));
		m_maxgfxelement++;
	}
}

void gcm394_base_video_device::device_start()
{
	for (uint8_t i = 0; i < 32; i++)
	{
		m_rgb5_to_rgb8[i] = (i << 3) | (i >> 2);
	}
	for (uint16_t i = 0; i < 0x8000; i++)
	{
		m_rgb555_to_rgb888[i] = (m_rgb5_to_rgb8[(i >> 10) & 0x1f] << 16) |
								(m_rgb5_to_rgb8[(i >>  5) & 0x1f] <<  8) |
								(m_rgb5_to_rgb8[(i >>  0) & 0x1f] <<  0);
	}

	m_video_irq_cb.resolve();

	m_maxgfxelement = 0;

	// debug helper only
	if (memregion(":maincpu"))
		decodegfx(":maincpu");

	m_space_read_cb.resolve_safe(0);

	m_screenpos_timer = timer_alloc(TIMER_SCREENPOS);
	m_screenpos_timer->adjust(attotime::never);


	save_item(NAME(m_screenbuf));
	save_item(NAME(m_rgb5_to_rgb8));
	save_item(NAME(m_rgb555_to_rgb888));
	save_item(NAME(m_page0_addr_lsb));
	save_item(NAME(m_page0_addr_msb));
	save_item(NAME(m_page1_addr_lsb));
	save_item(NAME(m_page1_addr_msb));
	save_item(NAME(m_707e_videodma_bank));
	save_item(NAME(m_videodma_size));
	save_item(NAME(m_videodma_dest));
	save_item(NAME(m_videodma_source));
	save_item(NAME(m_tmap0_regs));
	save_item(NAME(m_tmap1_regs));
	save_item(NAME(m_tmap2_regs));
	save_item(NAME(m_tmap3_regs));
	save_item(NAME(m_tmap0_scroll));
	save_item(NAME(m_tmap1_scroll));
	save_item(NAME(m_tmap2_scroll));
	save_item(NAME(m_tmap3_scroll));
	save_item(NAME(m_707f));
	save_item(NAME(m_703a_palettebank));
	save_item(NAME(m_video_irq_enable));
	save_item(NAME(m_video_irq_status));
	save_item(NAME(m_702a));
	save_item(NAME(m_7030_brightness));
	save_item(NAME(m_xirqpos));
	save_item(NAME(m_yirqpos));
	save_item(NAME(m_703c_tvcontrol1));
	save_item(NAME(m_7042_sprite));
	save_item(NAME(m_7080));
	save_item(NAME(m_7081));
	save_item(NAME(m_7082));
	save_item(NAME(m_7083));
	save_item(NAME(m_7084));
	save_item(NAME(m_7085));
	save_item(NAME(m_7086));
	save_item(NAME(m_7087));
	save_item(NAME(m_7088));
	save_item(NAME(m_sprite_7022_gfxbase_lsb));
	save_item(NAME(m_sprite_702d_gfxbase_msb));
	save_item(NAME(m_page2_addr_lsb));
	save_item(NAME(m_page2_addr_msb));
	save_item(NAME(m_page3_addr_lsb));
	save_item(NAME(m_page3_addr_msb));
	save_item(NAME(m_spriteram));
	save_item(NAME(m_spriteextra));
	save_item(NAME(m_paletteram));
	save_item(NAME(m_maxgfxelement));
	save_item(NAME(m_pal_displaybank_high));
	save_item(NAME(m_alt_tile_addressing));
}

void gcm394_base_video_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_tmap0_regs[i] = 0x0000;
		m_tmap1_regs[i] = 0x0000;
		m_tmap2_regs[i] = 0x0000;
		m_tmap3_regs[i] = 0x0000;
		m_tmap2_scroll[i] = 0x0000;
		m_tmap3_scroll[i] = 0x0000;
	}

	for (int i = 0; i < 2; i++)
	{
		m_tmap0_scroll[i] = 0x0000;
		m_tmap1_scroll[i] = 0x0000;
	}

	for (int i = 0; i < 0x400; i++)
	{
		m_spriteextra[i] = 0x0000;
		m_spriteram[i] = 0x0000;
	}

	for (int i=0;i<0x100 * 0x10;i++)
		m_paletteram[i] = machine().rand()&0x7fff;


	m_707f = 0x0000;
	m_703a_palettebank = 0x0000;
	m_video_irq_enable = 0x0000;
	m_video_irq_status = 0x0000;

	m_702a = 0x0000;
	m_7030_brightness = 0x0000;
	m_xirqpos = 0x0000;
	m_yirqpos = 0x0000;
	m_703c_tvcontrol1 = 0x0000;

	m_7042_sprite = 0x0000;

	m_7080 = 0x0000;
	m_7081 = 0x0000;
	m_7082 = 0x0000;
	m_7083 = 0x0000;
	m_7084 = 0x0000;
	m_7085 = 0x0000;
	m_7086 = 0x0000;
	m_7087 = 0x0000;
	m_7088 = 0x0000;

	m_707e_videodma_bank = 0x0000;
	m_videodma_size = 0x0000;
	m_videodma_dest = 0x0000;
	m_videodma_source = 0x0000;

	m_sprite_7022_gfxbase_lsb = 0;
	m_sprite_702d_gfxbase_msb = 0;
	m_page2_addr_lsb = 0;
	m_page2_addr_msb = 0;
	m_page3_addr_lsb = 0;
	m_page3_addr_msb = 0;

}

/*************************
*     Video Hardware     *
*************************/

template<gcm394_base_video_device::blend_enable_t Blend, gcm394_base_video_device::rowscroll_enable_t RowScroll, gcm394_base_video_device::flipx_t FlipX>
void gcm394_base_video_device::draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t bitmap_addr, uint32_t tile, int32_t h, int32_t w, uint8_t bpp, uint32_t yflipmask, uint32_t palette_offset, int addressing_mode)
{
	uint32_t nc_bpp = ((bpp) + 1) << 1;

	// probably don't do this here as this SoC has extended palette for higher bpp modes
	//palette_offset >>= nc_bpp;
	//palette_offset <<= nc_bpp;

	uint32_t bits_per_row = nc_bpp * w / 16;
	uint32_t words_per_tile;

	if (addressing_mode == 1)
	{
		words_per_tile = bits_per_row * h;
	}
	else
	{
		words_per_tile = 8; // seems to be correct for sprites regardless of size / bpp on smartfp
	}

	int x_max;
	if (m_707f & 0x0010)
	{
		x_max = 0x400;
	}
	else
	{
		x_max = 0x200;
	}


	uint32_t m = (bitmap_addr) + (words_per_tile * tile + bits_per_row * (line ^ yflipmask));


	uint32_t bits = 0;
	uint32_t nbits = 0;
	uint32_t y = line;

	int yy = (yoff + y);// &0x1ff;
	//if (yy >= 0x01c0)
	//  yy -= 0x0200;

	if (yy > cliprect.max_y || yy < 0)
		return;

	int y_index = yy * m_screen->width();

	for (int32_t x = FlipX ? (w - 1) : 0; FlipX ? x >= 0 : x < w; FlipX ? x-- : x++)
	{
		int xx = xoff + x;

		bits <<= nc_bpp;

		if (nbits < nc_bpp)
		{
			uint16_t b = m_space_read_cb((m++)&0x7ffffff); // smartfp suggests either 0x7ffffff mask, or some bits are being set incorrectly, jak_s500 needs over 0x3ffffff at least
			b = (b << 8) | (b >> 8);
			bits |= b << (nc_bpp - nbits);
			nbits += 16;
		}
		nbits -= nc_bpp;

		int pen = bits >> 16;

		int current_palette_offset = palette_offset;


		// for planes above 4bpp palette ends up being pulled from different places?
		if (nc_bpp < 6)
		{
			// 2bpp
			// 4bpp
			if (m_pal_displaybank_high) // how is this set?
				current_palette_offset |= 0x0800;

		}
		else if (nc_bpp < 8)
		{
			// 6bpp
		//  current_palette_offset |= 0x0800;

		}
		else
		{
			//pen = machine().rand() & 0x1f;
			// 8bpp
		}

		uint32_t pal = current_palette_offset + pen;
		bits &= 0xffff;

		if (RowScroll)
			xx -= 0;// (int16_t)m_scrollram[yy & 0x1ff];

		xx &= (x_max-1);
		if (xx >= (x_max-0x40))
			xx -= x_max;

		if (xx >= 0 && xx <= cliprect.max_x)
		{
			int pix_index = xx + y_index;

			uint16_t rgb = m_paletteram[pal];

			if (!(rgb & 0x8000))
			{
				if (Blend)
				{
					/*
					m_screenbuf[pix_index] = (mix_channel((uint8_t)(m_screenbuf[pix_index] >> 16), m_rgb5_to_rgb8[(rgb >> 10) & 0x1f]) << 16) |
					                         (mix_channel((uint8_t)(m_screenbuf[pix_index] >>  8), m_rgb5_to_rgb8[(rgb >> 5) & 0x1f]) << 8) |
					                         (mix_channel((uint8_t)(m_screenbuf[pix_index] >>  0), m_rgb5_to_rgb8[rgb & 0x1f]));
					*/
					m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
				}
				else
				{
					m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
				}
			}
		}
	}
}

void gcm394_base_video_device::draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs, uint16_t *scroll)
{
	uint32_t xscroll = scroll[0];
	uint32_t yscroll = scroll[1];
	uint32_t attr_reg = regs[0];
	uint32_t ctrl_reg = regs[1];
	uint32_t tilemap = regs[2];
	uint32_t palette_map = regs[3];
	address_space &space = m_cpu->space(AS_PROGRAM);

	if (!(ctrl_reg & PAGE_ENABLE_MASK))
	{
		return;
	}

	if (((attr_reg & PAGE_PRIORITY_FLAG_MASK) >> PAGE_PRIORITY_FLAG_SHIFT) != priority)
	{
		return;
	}

	if (ctrl_reg & 0x01) // bitmap mode jak_car2 and jak_s500 use for the ingame race sections, also have a bitmap test in test mode
	{
		if (ctrl_reg & 0x10)
			popmessage("bitmap mode %08x with rowscroll\n", bitmap_addr);
		else
			popmessage("bitmap mode %08x\n", bitmap_addr);

		// note, in interlace modes it appears every other line is unused? (480 entry table, but with blank values)
		// and furthermore the rowscroll and rowzoom tables only have 240 entries, not enough for every line
		// the end of the rowscroll table (entries 240-255) contain something else, maybe garbage data as it's offscreen, maybe not

		uint32_t linebase = space.read_word(tilemap + scanline); // every other word is unused, but there are only enough entries for 240 lines then, sometimes to do with interlace mode?
		uint16_t palette = space.read_word(palette_map + (scanline / 2));

		if (scanline & 1)
			palette >>= 8;
		else
			palette &= 0xff;

		if (!linebase)
			return;

		linebase = linebase | (palette << 16);

		// this logic works for jak_s500 and the test modes to get the correct base, doesn't seem to work for jak_car2 ingame, maybe data is copied to wrong place?
		int gfxbase = (bitmap_addr&0x7ffffff) + (linebase&0x7ffffff);

		for (int i = 0; i < 160; i++) // will have to be 320 for jak_car2 ingame, jak_s500 lines are wider than screen, and zoomed
		{
			uint16_t pix = m_space_read_cb((gfxbase++)&0x7ffffff);
			int xx;
			int y_index = scanline * m_screen->width();
			uint16_t pal;

			if ((scanline >= 0) && (scanline < 480))
			{
				xx = i * 2;

				pal = (pix & 0xff) | 0x100;

				if (xx >= 0 && xx <= cliprect.max_x)
				{
					int pix_index = xx + y_index;

					uint16_t rgb = m_paletteram[pal];

					if (!(rgb & 0x8000))
					{
						m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
					}
				}

				xx = (i * 2)+1;
				pal = (pix >> 8) + 0x100;

				if (xx >= 0 && xx <= cliprect.max_x)
				{
					int pix_index = xx + y_index;

					uint16_t rgb = m_paletteram[pal];

					if (!(rgb & 0x8000))
					{
						m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
					}
				}
			}
		}
	}
	else
	{
		uint32_t tile_h = 8 << ((attr_reg & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
		uint32_t tile_w = 8 << ((attr_reg & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

		int total_width;
		int use_alt_drawmode = m_alt_tile_addressing;
		int y_mask = 0;

		// just a guess based on this being set on the higher resolution tilemaps we've seen, could be 100% incorrect register
		if ((attr_reg >> 14) & 0x2)
		{
			total_width = 1024;
			y_mask = 0x1ff;
		//  use_alt_drawmode = 1; // probably doesn't control this
		}
		else
		{
			total_width = 512;
			y_mask = 0xff;
		//  use_alt_drawmode = 0; // probably doesn't control this
		}

		uint32_t tile_count_x = total_width / tile_w;

		uint32_t bitmap_y = (scanline + yscroll) & y_mask;
		uint32_t y0 = bitmap_y / tile_h;
		uint32_t tile_scanline = bitmap_y % tile_h;
		uint32_t tile_address = tile_count_x * y0;

		for (uint32_t x0 = 0; x0 < tile_count_x; x0++, tile_address++)
		{
			uint32_t yy = ((tile_h * y0 - yscroll + 0x10) & y_mask) - 0x10;
			uint32_t xx = (tile_w * x0 - xscroll) & (total_width-1);
			uint32_t tile = (ctrl_reg & PAGE_WALLPAPER_MASK) ? space.read_word(tilemap) : space.read_word(tilemap + tile_address);



			if (!tile)
				continue;


			uint32_t tileattr = attr_reg;
			uint32_t tilectrl = ctrl_reg;


			bool blend;
			bool row_scroll;
			bool flip_x;
			uint32_t yflipmask;
			uint32_t palette_offset;

			blend = (tileattr & 0x4000 || tilectrl & 0x0100);
			row_scroll = (tilectrl & 0x0010);


			if ((ctrl_reg & 2) == 0) // RegSet:0
			{
				uint16_t palette = (ctrl_reg & PAGE_WALLPAPER_MASK) ? space.read_word(palette_map) : space.read_word(palette_map + tile_address / 2);
				if (x0 & 1)
					palette >>= 8;

				flip_x = palette & 0x0010;
				yflipmask = (palette & 0x0020) ? tile_h - 1 : 0;
				palette_offset = (palette & 0x0f) << 4;

				//tilectrl &= ~0x0100;
				//tilectrl |= (palette << 2) & 0x0100;    // blend
			}
			else // RegSet:1
			{
				if (m_alt_tile_addressing == 0)
				{
					// smartfp needs the attribute table to contain extra tile bits even if regset is 1
					uint16_t palette = (ctrl_reg & PAGE_WALLPAPER_MASK) ? space.read_word(palette_map) : space.read_word(palette_map + tile_address / 2);
					if (x0 & 1)
						palette >>= 8;

					tile |= (palette & 0x0007) << 16;

					flip_x = (tileattr & TILE_X_FLIP);
					yflipmask = tileattr & TILE_Y_FLIP ? tile_h - 1 : 0;
					palette_offset = (tileattr & 0x0f00) >> 4;

				}
				else
				{
					flip_x = (tileattr & TILE_X_FLIP);
					yflipmask = tileattr & TILE_Y_FLIP ? tile_h - 1 : 0;
					palette_offset = (tileattr & 0x0f00) >> 4;
				}
			}


			//palette_offset |= 0x0900;
			palette_offset |= m_pal_back;

			const uint8_t bpp = tileattr & 0x0003;


			if (blend)
			{
				if (row_scroll)
				{
					if (flip_x)
						draw<BlendOn, RowScrollOn, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
					else
						draw<BlendOn, RowScrollOn, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
				}
				else
				{
					if (flip_x)
						draw<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
					else
						draw<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
				}
			}
			else
			{
				if (row_scroll)
				{
					if (flip_x)
						draw<BlendOff, RowScrollOn, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
					else
						draw<BlendOff, RowScrollOn, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
				}
				else
				{
					if (flip_x)
						draw<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
					else
						draw<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_scanline, xx, yy, bitmap_addr, tile, tile_h, tile_w, bpp, yflipmask, palette_offset, use_alt_drawmode);
				}
			}
		}
	}
}


void gcm394_base_video_device::draw_sprite(const rectangle& cliprect, uint32_t scanline, int priority, uint32_t base_addr)
{
	uint32_t bitmap_addr = (m_sprite_702d_gfxbase_msb << 16) | m_sprite_7022_gfxbase_lsb;
	uint32_t tile = m_spriteram[base_addr + 0];
	int16_t x = m_spriteram[base_addr + 1];
	int16_t y = m_spriteram[base_addr + 2];
	uint16_t attr = m_spriteram[base_addr + 3];


	if (!tile) // this check needs to come before the additional attribute bits are added in? (smartfp title)
	{
		return;
	}

	int addressing_mode = 0;

	int screenwidth, screenheight, x_max;
	if (m_707f & 0x0010)
	{
		screenwidth = 640;
		screenheight = 480;
		x_max = 0x400;
	}
	else
	{
		screenwidth = 320;
		screenheight = 240;
		x_max = 0x200;
	}


	// good for gormiti, smartfp, wrlshunt, paccon, jak_totm, jak_s500, jak_gtg
	if ((m_7042_sprite & 0x0010) == 0x10)
		addressing_mode = 0; // smartfp, paccon
	else
		addressing_mode = 1;

	if (addressing_mode == 0) // smartfp, paccon
		tile |= m_spriteextra[base_addr / 4] << 16;

	if (((attr & PAGE_PRIORITY_FLAG_MASK) >> PAGE_PRIORITY_FLAG_SHIFT) != priority)
	{
		return;
	}

	const uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	const uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);



	if (!(m_7042_sprite & SPRITE_COORD_TL_MASK))
	{
		x = ((screenwidth / 2) + x) - w / 2;
		y = ((screenheight / 2) - y) - (h / 2) + 8;
	}


	x &= (x_max - 1);
	y &= 0x01ff;

	uint32_t tile_line = ((scanline - y) + 0x200) % h;
	int16_t test_y = (y + tile_line) & 0x1ff;
	if (test_y >= 0x01c0)
		test_y -= 0x0200;

	if (test_y != scanline)
	{
		return;
	}

	bool blend = (attr & 0x4000);

	bool flip_x;
	uint8_t bpp;
	uint32_t yflipmask;
	uint32_t palette_offset;

	// different attribute use?
	if (screenwidth == 320)
	{
		flip_x = (attr & TILE_X_FLIP);
		bpp = attr & 0x0003;
		yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;
		palette_offset = (attr & 0x0f00) >> 4;
	}
	else
	{
		flip_x = 0;// (attr & TILE_X_FLIP);
		bpp = attr & 0x0003;
		yflipmask = 0;// attr& TILE_Y_FLIP ? h - 1 : 0;
		palette_offset = (attr & 0x0f00) >> 4;
	}

	//palette_offset |= 0x0d00;
	palette_offset |= m_pal_sprites;

	if (blend)
	{
		if (flip_x)
			draw<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset, addressing_mode);
		else
			draw<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset, addressing_mode);
	}
	else
	{
		if (flip_x)
			draw<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset, addressing_mode);
		else
			draw<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, bitmap_addr, tile, h, w, bpp, yflipmask, palette_offset, addressing_mode);
	}
}

void gcm394_base_video_device::draw_sprites(const rectangle &cliprect, uint32_t scanline, int priority)
{
	// paccon suggests this, does older hardware have similar?
	int numsprites = (m_7042_sprite & 0xff00) >> 8;
	if (numsprites == 0)
		numsprites = 0x100;

	for (uint32_t n = 0; n < numsprites; n++)
	{
		draw_sprite(cliprect, scanline, priority, 4 * n);
	}
}

uint32_t gcm394_base_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// For jak_car2 and jak_gtg the palette entry for 'magenta' in the test mode is intentionally set to a transparent black pen
	// (it is stored in the palette table in ROM that way, and copied directly) so the only way for the magenta entries on the screen
	// to be correctly displayed is if there is a magenta BG pen to fall through to (or for another palette write to change the palette
	// that is copied, but this does not appear to be the case).  How the bg pen is set is unknown, it is not a regular palette entry.
	// The 'bitmap test mode' in jak_car2 requires this to be black instead.

	// jak_s500 briely sets pen 0 of the layer to magenta, but then ends up erasing it

	//const uint16_t bgcol = 0x7c1f; // magenta
	const uint16_t bgcol = 0x0000; // black


	if (m_707f & 0x0010)
	{
		m_screen->set_visible_area(0, 640-1, 0, 480-1);
	}
	else
	{
		m_screen->set_visible_area(0, 320-1, 0, 240-1);
	}

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		uint32_t* bufferline = &m_screenbuf[scanline * m_screen->width()];

		for (int x = 0; x < m_screen->width(); x++)
		{
			bufferline[x] = m_rgb555_to_rgb888[bgcol];
		}

		for (int i = 0; i < 4; i++)
		{
			const int draw_all = 1;

			if (1)
			{
				if ((!(machine().input().code_pressed(KEYCODE_Q))) || draw_all) draw_page(cliprect, scanline, i, (m_page0_addr_lsb | (m_page0_addr_msb<<16)), m_tmap0_regs, m_tmap0_scroll);
				if ((!(machine().input().code_pressed(KEYCODE_W))) || draw_all) draw_page(cliprect, scanline, i, (m_page1_addr_lsb | (m_page1_addr_msb<<16)), m_tmap1_regs, m_tmap1_scroll);
				if ((!(machine().input().code_pressed(KEYCODE_E))) || draw_all) draw_page(cliprect, scanline, i, (m_page2_addr_lsb | (m_page2_addr_msb<<16)), m_tmap2_regs, m_tmap2_scroll);
				if ((!(machine().input().code_pressed(KEYCODE_R))) || draw_all) draw_page(cliprect, scanline, i, (m_page3_addr_lsb | (m_page3_addr_msb<<16)), m_tmap3_regs, m_tmap3_scroll);

			}
			if ((!(machine().input().code_pressed(KEYCODE_T))) || draw_all) draw_sprites(cliprect, scanline, i);
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
		uint32_t *src = &m_screenbuf[cliprect.min_x + m_screen->width() * y];
		memcpy(dest, src, sizeof(uint32_t) * ((cliprect.max_x - cliprect.min_x) + 1));
	}

	return 0;
}


void gcm394_base_video_device::write_tmap_scroll(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page X scroll
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d X Scroll = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	case 0x1: // Page Y scroll
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Y Scroll = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;
	}
}

void gcm394_base_video_device::write_tmap_regs(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page Attributes
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Attributes = %04x (unk %01x: Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n",  machine().describe_context(), tmap, data,
			(data & 0xc000) >> 14, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		regs[offset] = data;
		break;

	case 0x1: // Page Control
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Control = %04x (unk:%02x Blend:%d, HiColor:%d, unk:%d, unk%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n",  machine().describe_context(), tmap, data,
			(data & 0xfe00) >> 9, BIT(data, 8), BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		regs[offset] = data;
		break;

	case 0x2: // Page Tile Address
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Tile Address = %04x\n",  machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	case 0x3: // Page Attribute Address
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d Attribute Address = %04x\n",  machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;
	}
}


// offsets 0,1,4,5,6,7 used in main IRQ code
// offsets 2,3 only cleared on startup

// Based on code analysis this seems to be the same as the regular tilemap regs, except for the addition of regs 2,3 which shift the remaining ones along.
// As the hardware appears to support ROZ these are probably 2 extra tile layers, with the 2 additional words being the ROZ parameters?


void gcm394_base_video_device::write_tmap_extrascroll(int tmap, uint16_t* regs, int offset, uint16_t data)
{
	switch (offset)
	{
	case 0x0: // Page X scroll
	case 0x1: // Page Y scroll
		write_tmap_scroll(tmap, regs, offset, data);
		break;

	case 0x2: //
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d X Unk Rotation Zoom Attribute1 = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	case 0x3:
		LOGMASKED(LOG_GCM394_TMAP, "%s: write_tmap_regs: Page %d X Unk Rotation Zoom Attribute = %04x\n", machine().describe_context(), tmap, data);
		regs[offset] = data;
		break;

	}
}


// **************************************** TILEMAP 0 *************************************************

READ16_MEMBER(gcm394_base_video_device::tmap0_regs_r)
{
	if (offset < 2)
	{
		return m_tmap0_scroll[offset];
	}
	else
	{
		return m_tmap0_regs[offset-2];
	}
}

WRITE16_MEMBER(gcm394_base_video_device::tmap0_regs_w)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap0_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 2)
	{
		write_tmap_scroll(0, m_tmap0_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(0, m_tmap0_regs, offset-2, data);
	}
}

READ16_MEMBER(gcm394_base_video_device::tmap0_tilebase_lsb_r)
{
	return m_page0_addr_lsb;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap0_tilebase_lsb_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page0_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap0 tilegfxbase is now %04x%04x)\n", m_page0_addr_msb, m_page0_addr_lsb);
}

READ16_MEMBER(gcm394_base_video_device::tmap0_tilebase_msb_r)
{
	return m_page0_addr_msb;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap0_tilebase_msb_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap0_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page0_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap0 tilegfxbase is now %04x%04x)\n", m_page0_addr_msb, m_page0_addr_lsb);
}

// **************************************** TILEMAP 1 *************************************************


READ16_MEMBER(gcm394_base_video_device::tmap1_regs_r)
{
	if (offset < 2)
	{
		return m_tmap1_scroll[offset];
	}
	else
	{
		return m_tmap1_regs[offset-2];
	}
}

WRITE16_MEMBER(gcm394_base_video_device::tmap1_regs_w)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap1_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 2)
	{
		write_tmap_scroll(1, m_tmap1_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(1, m_tmap1_regs, offset-2, data);
	}
}

READ16_MEMBER(gcm394_base_video_device::tmap1_tilebase_lsb_r)
{
	return m_page1_addr_lsb;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap1_tilebase_lsb_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page1_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap1 tilegfxbase is now %04x%04x)\n", m_page1_addr_msb, m_page1_addr_lsb);
}

READ16_MEMBER(gcm394_base_video_device::tmap1_tilebase_msb_r)
{
	return m_page1_addr_msb;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap1_tilebase_msb_w)
{
	LOGMASKED(LOG_GCM394_TMAP, "%s:gcm394_base_video_device::tmap1_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page1_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(tmap1 tilegfxbase is now %04x%04x)\n", m_page1_addr_msb, m_page1_addr_lsb);
}

// **************************************** unknown video device 1 (another tilemap? roz? line? zooming sprite layer?) *************************************************

READ16_MEMBER(gcm394_base_video_device::tmap2_regs_r)
{
	if (offset < 4)
	{
		return m_tmap2_scroll[offset];
	}
	else
	{
		return m_tmap2_regs[offset-4];
	}
}

WRITE16_MEMBER(gcm394_base_video_device::tmap2_regs_w)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap2_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 4)
	{
		write_tmap_extrascroll(2, m_tmap2_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(2, m_tmap2_regs, offset-4, data);
	}
}

READ16_MEMBER(gcm394_base_video_device::tmap2_tilebase_lsb_r)
{
	return m_page2_addr_lsb;
}


WRITE16_MEMBER(gcm394_base_video_device::tmap2_tilebase_lsb_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap2_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page2_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid1 tilegfxbase is now %04x%04x)\n", m_page2_addr_msb, m_page2_addr_lsb);
}

READ16_MEMBER(gcm394_base_video_device::tmap2_tilebase_msb_r)
{
	return m_page2_addr_msb;
}

WRITE16_MEMBER(gcm394_base_video_device::tmap2_tilebase_msb_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap2_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page2_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid1 tilegfxbase is now %04x%04x)\n", m_page2_addr_msb, m_page2_addr_lsb);
}

// **************************************** unknown video device 2 (another tilemap? roz? lines? zooming sprite layer?) *************************************************

READ16_MEMBER(gcm394_base_video_device::tmap3_regs_r)
{
	if (offset < 4)
	{
		return m_tmap3_scroll[offset];
	}
	else
	{
		return m_tmap3_regs[offset-4];
	}
}

WRITE16_MEMBER(gcm394_base_video_device::tmap3_regs_w)
{
	LOGMASKED(LOG_GCM394_TMAP_EXTRA, "%s:gcm394_base_video_device::tmap3_regs_w %01x %04x\n", machine().describe_context(), offset, data);
	if (offset < 4)
	{
		write_tmap_extrascroll(3, m_tmap3_scroll, offset, data);
	}
	else
	{
		write_tmap_regs(3, m_tmap3_regs, offset-4, data);
	}
}

READ16_MEMBER(gcm394_base_video_device::tmap3_tilebase_lsb_r)
{
	return m_page3_addr_lsb;
}


WRITE16_MEMBER(gcm394_base_video_device::tmap3_tilebase_lsb_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap3_tilebase_lsb_w %04x\n", machine().describe_context(), data);
	m_page3_addr_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid2 tilegfxbase is now %04x%04x)\n", m_page3_addr_msb, m_page3_addr_lsb);
}

READ16_MEMBER(gcm394_base_video_device::tmap3_tilebase_msb_r)
{
	return m_page3_addr_msb;
}


WRITE16_MEMBER(gcm394_base_video_device::tmap3_tilebase_msb_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::tmap3_tilebase_msb_w %04x\n", machine().describe_context(), data);
	m_page3_addr_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(unk_vid2 tilegfxbase is now %04x%04x)\n", m_page3_addr_msb, m_page3_addr_lsb);
}

// **************************************** sprite control registers *************************************************

// set to 001264c0 in wrlshunt, which point at the menu selectors (game names, arrows etc.)

READ16_MEMBER(gcm394_base_video_device::sprite_7022_gfxbase_lsb_r)
{
	return m_sprite_7022_gfxbase_lsb;
}

WRITE16_MEMBER(gcm394_base_video_device::sprite_7022_gfxbase_lsb_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_7022_gfxbase_lsb_w %04x\n", machine().describe_context(), data);
	m_sprite_7022_gfxbase_lsb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(sprite tilebase is now %04x%04x)\n", m_sprite_702d_gfxbase_msb, m_sprite_7022_gfxbase_lsb);
}

READ16_MEMBER(gcm394_base_video_device::sprite_702d_gfxbase_msb_r)
{
	return m_sprite_702d_gfxbase_msb;
}

WRITE16_MEMBER(gcm394_base_video_device::sprite_702d_gfxbase_msb_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_702d_gfxbase_msb_w %04x\n", machine().describe_context(), data);
	m_sprite_702d_gfxbase_msb = data;
	LOGMASKED(LOG_GCM394_TMAP, "\t(sprite tilebase tilegfxbase is now %04x%04x)\n", m_sprite_702d_gfxbase_msb, m_sprite_7022_gfxbase_lsb);
}

READ16_MEMBER(gcm394_base_video_device::sprite_7042_extra_r)
{
	uint16_t retdata = m_7042_sprite;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_7042_extra_r (returning: %04x)\n", machine().describe_context(), retdata);
	return retdata;
}

WRITE16_MEMBER(gcm394_base_video_device::sprite_7042_extra_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::sprite_7042_extra_w %04x\n", machine().describe_context(), data);
	m_7042_sprite = data;
	//popmessage("extra modes %04x\n", data);
}


// **************************************** video DMA device *************************************************

WRITE16_MEMBER(gcm394_base_video_device::video_dma_source_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_source_w %04x\n", machine().describe_context(), data);
	m_videodma_source = data;
}

WRITE16_MEMBER(gcm394_base_video_device::video_dma_dest_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_dest_w %04x\n", machine().describe_context(), data);
	m_videodma_dest = data;
}

READ16_MEMBER(gcm394_base_video_device::video_dma_size_busy_r)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_size_busy_r\n", machine().describe_context());
	return 0x0000;
}

WRITE16_MEMBER(gcm394_base_video_device::video_dma_size_trigger_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s:gcm394_base_video_device::video_dma_size_trigger_w %04x\n", machine().describe_context(), data);
	m_videodma_size = data;

	LOGMASKED(LOG_GCM394_VIDEO_DMA, "%s: doing sprite / video DMA source %04x dest %04x size %04x value of 707e (bank) %04x value of 707f %04x\n", machine().describe_context(), m_videodma_source, m_videodma_dest, m_videodma_size, m_707e_videodma_bank, m_707f );

	for (int i = 0; i <= m_videodma_size; i++)
	{
		uint16_t dat = space.read_word(m_videodma_source+i);
		space.write_word(m_videodma_dest + i, dat);
	}

	m_videodma_size = 0x0000;

	if (m_video_irq_enable & 4)
	{
		const uint16_t old = m_video_irq_status;
		m_video_irq_status |= 4;
		const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
		if (changed)
			check_video_irq();
	}
}

WRITE16_MEMBER(gcm394_base_video_device::video_dma_unk_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_dma_unk_w %04x\n", machine().describe_context(), data);
	m_707e_videodma_bank = data;
}

READ16_MEMBER(gcm394_base_video_device::video_707c_r)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707c_r\n", machine().describe_context());
	return 0x8000;
}

/* 707f is VERY important, lots of rendering codepaths in the code depend on the value it returns.

   all operations in the code based on 707f are bit based, usually read register, set / clear a bit
   and then write register, or read register and test an individual bit.

   our current codeflow means that bits are only ever set, not cleared.

   are the bits triggers? acks? enables? status flags?

   in wrlshunt this ends up being set to  02f9   ---- --x- xxxx x--x
   and in smartfp it ends up being set to 0065   ---- ---- -xx- -x-x

   is this because wrlshunt uses more layers?
*/


READ16_MEMBER(gcm394_base_video_device::video_707f_r)
{
	uint16_t retdata = m_707f;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707f_r (returning %04x)\n", machine().describe_context(), retdata);
	return retdata;
}
WRITE16_MEMBER(gcm394_base_video_device::video_707f_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_707f_w %04x\n", machine().describe_context(), data);

	for (int i = 0; i < 16; i++)
	{
		uint16_t mask = 1 << i;

		if ((m_707f & mask) != (data & mask))
		{
			if (data & mask)
			{
				LOGMASKED(LOG_GCM394_VIDEO, "\tbit %04x Low -> High\n", mask);
			}
			else
			{
				LOGMASKED(LOG_GCM394_VIDEO, "\tbit %04x High -> Low\n", mask);
			}
		}
	}

	m_707f = data;

	//popmessage("707f is %04x\n", data);
}

READ16_MEMBER(gcm394_base_video_device::video_703a_palettebank_r)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703a_palettebank_r\n", machine().describe_context());
	return m_703a_palettebank;
}

WRITE16_MEMBER(gcm394_base_video_device::video_703a_palettebank_w)
{
	// I don't think bit 0 (0x01) is a bank select, it might be a 'mode select' for how the palette operates.
	// neither lazertag or tkmag220 set it
	// lazertag uses 2 banks (0 and 8)
	// tkmag220 only uses 1 bank (0)

	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703a_palettebank_w %04x\n", machine().describe_context(), data);
	m_703a_palettebank = data;
}

READ16_MEMBER(gcm394_base_video_device::videoirq_source_enable_r)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::videoirq_source_enable_r\n", machine().describe_context());
	return m_video_irq_enable;
}

WRITE16_MEMBER(gcm394_base_video_device::videoirq_source_enable_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "videoirq_source_enable_w: Video IRQ Enable = %04x (DMA:%d, Timing:%d, Blanking:%d)\n", data, BIT(data, 2), BIT(data, 1), BIT(data, 0));
	const uint16_t old = m_video_irq_enable & m_video_irq_status;
	m_video_irq_enable = data & 0x0007;
	const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
	if (changed)
		check_video_irq();
}

READ16_MEMBER(gcm394_base_video_device::video_7063_videoirq_source_r)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7063_videoirq_source_r\n", machine().describe_context());
	return m_video_irq_status;
}


WRITE16_MEMBER(gcm394_base_video_device::video_7063_videoirq_source_ack_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "video_7063_videoirq_source_ack_w: Video IRQ Acknowledge = %04x\n", data);
	const uint16_t old = m_video_irq_enable & m_video_irq_status;
	m_video_irq_status &= ~data;
	const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
	if (changed)
		check_video_irq();
}

WRITE16_MEMBER(gcm394_base_video_device::video_702a_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_702a_w %04x\n", machine().describe_context(), data);
	m_702a = data;
}

READ16_MEMBER(gcm394_base_video_device::video_curline_r)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s: video_r: Current Line: %04x\n", machine().describe_context(), m_screen->vpos());
	return m_screen->vpos();
}

// read in IRQ
READ16_MEMBER(gcm394_base_video_device::video_7030_brightness_r)
{
	/* wrlshunt ends up doing an explicit jump to 0000 shortly after boot if you just return the value written here, however I think that is correct code flow and something else is wrong
	   as this simply looks like some kind of brightness register - there is code to decrease it from 0xff to 0x00 by 0x5 increments (waiting for it to hit 0x05) and code to do the reverse
	   either way it really looks like the data written should be read back.
	*/
	uint16_t retdat = m_7030_brightness;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7030_brightness_r (returning %04x)\n", machine().describe_context(), retdat);
	return retdat;
}

WRITE16_MEMBER(gcm394_base_video_device::video_7030_brightness_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7030_brightness_w %04x\n", machine().describe_context(), data);
	m_7030_brightness = data;
}

void gcm394_base_video_device::update_raster_split_position()
{
	// this might need updating to handle higher res modes
	LOGMASKED(LOG_GCM394_VIDEO, "update_raster_split_position: %04x,%04x\n", m_yirqpos, m_xirqpos);
	if (m_xirqpos < 300 && m_yirqpos < 240)
	{
		m_screenpos_timer->adjust(m_screen->time_until_pos(m_yirqpos, m_xirqpos));
		//printf("setting irq timer for y:%d x:%d", m_yirqpos, m_xirqpos);
	}
	else
		m_screenpos_timer->adjust(attotime::never);
}

WRITE16_MEMBER(gcm394_base_video_device::split_irq_ypos_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:split_irq_ypos_w %04x\n", machine().describe_context(), data);

	m_yirqpos = data & 0x1ff;
	update_raster_split_position();
}

WRITE16_MEMBER(gcm394_base_video_device::split_irq_xpos_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:split_irq_xpos_w %04x\n", machine().describe_context(), data);

	m_xirqpos = data & 0x1ff;
	update_raster_split_position();
}

READ16_MEMBER(gcm394_base_video_device::video_703c_tvcontrol1_r)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703c_tvcontrol1_r\n", machine().describe_context());
	return m_703c_tvcontrol1;
}

WRITE16_MEMBER(gcm394_base_video_device::video_703c_tvcontrol1_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_703c_tvcontrol1_w %04x\n", machine().describe_context(), data);
	m_703c_tvcontrol1 = data;
}

READ16_MEMBER(gcm394_base_video_device::video_7051_r)
{
	/* related to what ends up crashing wrlshunt? */
	uint16_t retdat = 0x03ff;
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7051_r (returning %04x)\n", machine().describe_context(), retdat);
	return retdat;
}

READ16_MEMBER(gcm394_base_video_device::video_70e0_r)
{
	uint16_t retdat = machine().rand();
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_70e0_r (returning %04x)\n", machine().describe_context(), retdat);
	return retdat;
}


// this block get set once, in a single function, could be important
WRITE16_MEMBER(gcm394_base_video_device::video_7080_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7080_w %04x\n", machine().describe_context(), data); m_7080 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7081_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7081_w %04x\n", machine().describe_context(), data); m_7081 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7082_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7082_w %04x\n", machine().describe_context(), data); m_7082 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7083_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7083_w %04x\n", machine().describe_context(), data); m_7083 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7084_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7084_w %04x\n", machine().describe_context(), data); m_7084 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7085_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7085_w %04x\n", machine().describe_context(), data); m_7085 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7086_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7086_w %04x\n", machine().describe_context(), data); m_7086 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7087_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7087_w %04x\n", machine().describe_context(), data); m_7087 = data; }
WRITE16_MEMBER(gcm394_base_video_device::video_7088_w) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7088_w %04x\n", machine().describe_context(), data); m_7088 = data; }

READ16_MEMBER(gcm394_base_video_device::video_7083_r) { LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_7083_r\n", machine().describe_context()); return m_7083; }

WRITE16_MEMBER(gcm394_base_video_device::spriteram_w)
{
	// transfers an additional word for each sprite with this bit set (smartfp) or an entire extra bank (wrlshunt)
	// wrlshunt instead seems to base if it writes the extra data based on 707f so maybe this is more complex than banking

	// however for 707e only 0/1 is written, and it also gets written before system DMA, so despite being in the video DMA
	// region seems to operate separate from that.

	if (m_707e_videodma_bank == 0x0000)
	{
		m_spriteram[offset] = data;
	}
	else if (m_707e_videodma_bank == 0x0001)
	{
		m_spriteextra[offset] = data;
	}
	else
	{
		LOGMASKED(LOG_GCM394_VIDEO, "%s: spriteram_w %04x %04x unknown bank %04x\n", machine().describe_context(), offset, data, m_707e_videodma_bank);
	}
}

READ16_MEMBER(gcm394_base_video_device::spriteram_r)
{
	if (m_707e_videodma_bank == 0x0000)
	{
		return m_spriteram[offset];
	}
	else if (m_707e_videodma_bank == 0x0001)
	{
		return m_spriteextra[offset];
	}
	else
	{
		LOGMASKED(LOG_GCM394_VIDEO, "%s: spriteram_r %04x unknown bank %04x\n", machine().describe_context(), offset,  m_707e_videodma_bank);
		return 0x0000;
	}
}

WRITE16_MEMBER(gcm394_base_video_device::palette_w)
{
	LOGMASKED(LOG_GCM394_VIDEO_PALETTE, "%s:gcm394_base_video_device::palette_w %04x : %04x (value of 0x703a is %04x)\n", machine().describe_context(), offset, data, m_703a_palettebank);

	if (m_703a_palettebank & 0xfff0)
	{
		fatalerror("palette writes with m_703a_palettebank %04x\n", m_703a_palettebank);
	}
	else
	{
		offset |= (m_703a_palettebank & 0x000f) << 8;

		m_paletteram[offset] = data;

		uint32_t pal = m_rgb555_to_rgb888[data & 0x7fff];
		int r = (pal >> 16) & 0xff;
		int g = (pal >> 8) & 0xff;
		int b = (pal >> 0) & 0xff;

		m_palette->set_pen_color(offset, rgb_t(r, g, b));
	}
}

READ16_MEMBER(gcm394_base_video_device::palette_r)
{
	if (m_703a_palettebank & 0xfff0)
	{
		fatalerror("palette read with m_703a_palettebank %04x\n", m_703a_palettebank);
	}
	else
	{
		offset |= (m_703a_palettebank & 0x000f) << 8;
		return m_paletteram[offset];
	}
}

WRITE16_MEMBER(gcm394_base_video_device::video_701c_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_701c_w (unknown video reg?) %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(gcm394_base_video_device::video_701d_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_701d_w (unknown video reg?) %04x\n", machine().describe_context(), data);
}

WRITE16_MEMBER(gcm394_base_video_device::video_701e_w)
{
	LOGMASKED(LOG_GCM394_VIDEO, "%s:gcm394_base_video_device::video_701e_w (unknown video reg?) %04x\n", machine().describe_context(), data);
}


void gcm394_base_video_device::check_video_irq()
{
	LOGMASKED(LOG_GCM394_VIDEO, "%ssserting Video IRQ (%04x, %04x)\n", (m_video_irq_status & m_video_irq_enable) ? "A" : "Dea", m_video_irq_status, m_video_irq_enable);
	m_video_irq_cb((m_video_irq_status & m_video_irq_enable) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(gcm394_base_video_device::vblank)
{
	if (!state)
	{
		m_video_irq_status &= ~1;
		LOGMASKED(LOG_GCM394_VIDEO, "Setting video IRQ status to %04x\n", m_video_irq_status);
		check_video_irq();
		return;
	}

	if (m_video_irq_enable & 1)
	{
		m_video_irq_status |= 1;
		LOGMASKED(LOG_GCM394_VIDEO, "Setting video IRQ status to %04x\n", m_video_irq_status);
		check_video_irq();
	}
}

void gcm394_base_video_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SCREENPOS:
		{
			if (m_video_irq_enable & 2)
			{
				m_video_irq_status |= 2;
				check_video_irq();
			}

			//printf("firing irq timer\n");

			m_screen->update_partial(m_screen->vpos());

			// fire again, jak_dbz pinball needs this
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_yirqpos, m_xirqpos));
			break;
		}
	}
}


static GFXDECODE_START( gfx )
GFXDECODE_END

void gcm394_base_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256*0x10);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx);

}


