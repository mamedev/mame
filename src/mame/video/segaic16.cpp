// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

****************************************************************************

    Hang On
    -------
        Control Board (834-5668):
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5049 (x2)  -- tilemaps
            315-5107 (PAL x2) -- horizontal timing control
            315-5108       -- vertical timing control
            315-5122 (PAL) -- timing

    Enduro Racer
    ------------
        CPU Side (171-5319):
            315-5164 (PAL)
            315-5165 (PAL)
            315-5166 (PAL)
            315-5167 (PAL)

        Video Side (171-5320):
            315-5049 (x2)  -- tilemaps
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5106 (PAL)
            315-5107 (PAL)
            315-5108 (PAL)
            315-5168 (PAL)
            315-5170 (PAL)
            315-5171 (PAL)
            315-5172 (PAL)

    Pre-System 16
    -------------
        Main Board (171-5335):
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5049 (x2)  -- tilemaps
            315-5107 (PAL) -- display timing
            315-5108 (PAL) -- display timing
            315-5141 (PAL) -- Z80 address decoding
            315-5143 (PAL) -- sprite-related?
            315-5144 (PAL) -- sprite-related?
            315-5147 (PAL) -- unknown, DTACK-related
            315-5149 (PAL) -- video mixing
            315-5193 (PAL) -- 68000/MCU interface & address decoding
            315-5202 (PAL) -- 68000/MCU interface & address decoding

    Sega System 16A
    ---------------
        Bottom Board (171-5307):
            315-5011       -- sprite line comparitor
            315-5012       -- sprite generator control
            315-5049 (x2)  -- tilemaps
            315-5107 (PAL) -- display timing
            315-5108 (PAL) -- display timing
            315-5143 (PAL) -- sprite-related?
            315-5144 (PAL) -- sprite-related?
            315-5145 (PAL)

        Top Board (171-5306):
            315-5141 (PAL) -- Z80 address decoding
            315-5142 (PAL)
            315-5149 (PAL) -- video mixing
            315-5150 (PAL)

    Sega System 16B
    ---------------
        Main Board (171-5357):
            315-5195       -- memory mapper
            315-5196       -- sprite generator
            315-5197       -- tilemap generator
            315-5213 (PAL) -- sprite-related
            315-5214 (PAL) -- unknown

        ROM Board (171-5521):
            315-5298 (PAL)

        ROM Board (171-5704):
            315-5298 (PAL)

        ROM Board (171-5797):
            315-5248       -- hardware multiplier
            315-5250       -- compare/timer
            315-5298 (PAL)

    Sega System 18
    --------------
        Main Board (171-5873B):
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5313       -- VDP
            315-5360       -- memory mapper?
            315-5361       -- sprite generator
            315-5362       -- tilemap generator
            315-5373 (PAL) -- video mixing
            315-5374 (PAL) -- sprite timing
            315-5375 (PAL) -- system timing
            315-5389 (PAL) -- VDP sync
            315-5390 (PAL)
            315-5391 (PAL) -- Z80 address decoding

        Main Board (171-5873-02B):
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5313       -- VDP
            315-5360       -- memory mapper?
            315-5361       -- sprite generator
            315-5362       -- tilemap generator
            315-5374 (PAL) -- sprite timing
            315-5375 (PAL) -- system timing
            315-5389 (PAL) -- VDP sync
            315-5391 (PAL) -- Z80 address decoding
            315-5430 (PAL) -- video mixing

        ROM Board (171-5987A):
            315-5436       -- tile/sprite banking

    Sega System C
    -------------
        Main Board:
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5313       -- VDP
            315-5393 (PAL)
            315-5394 (PAL)
            315-5395 (PAL)

    Super Hang On
    -------------
        CPU Board 171-5376-01:
            315-5195       -- memory mapper
            315-5218       -- PCM sound controller
            315-5155 (PAL x2) -- road bit extraction
            315-5222 (PAL) -- road mixing
            315-5223a (PAL)
            315-5224 (PAL)
            315-5225 (PAL)
            315-5226 (PAL)

        VIDEO Board: (not the same as out run !) 171-5480
            315-5196       -- sprite generator
            315-5197       -- tilemap generator
            315-5213 (PAL) -- sprite-related
            315-5242       -- color encoder
            315-5251 (PAL)

    Out Run
    -------
        CPU Board 837-6063-01:
            315-5195       -- memory mapper
            315-5218       -- PCM sound controller
            315-5155 (PAL x2) -- road bit extraction
            315-5222 (PAL) -- road mixing
            315-5223a (PAL)
            315-5224 (PAL)
            315-5225 (PAL)
            315-5226 (PAL)

        VIDEO Board: 837-6064, 171-5377-01
            315-5197       -- tilemap generator
            315-5211       -- sprite generator
            315-5227a (PAL)
            315-5228 (PAL)
            315-5242       -- color encoder

    Sega System 32
    --------------
        Main Board (317-5964):
            315-5242       -- color encoder
            315-5296       -- I/O chip
            315-5385
            315-5386       -- tilemap generator
            315-5387       -- sprite generator
            315-5388       -- video mixing
            315-5441 (PAL)
            315-5476

    X-Board
    -------
        Main Board:
            315-5197       -- tilemap generator
            315-5211A      -- sprite generator
            315-5218       -- PCM sound controller
            315-5242       -- color encoder
            315-5248 (x2)  -- hardware multiplier
            315-5249 (x2)  -- hardware divider
            315-5250 (x2)  -- compare/timer
            315-5275       -- road generator
            315-5278 (PAL) -- sprite ROM bank control
            315-5279 (PAL) -- video mixing (Afterburner)
            315-5280 (PAL) -- Z80 address decoding
            315-5290 (PAL) -- main CPU address decoding
            315-5291 (PAL) -- main CPU address decoding
            315-5304 (PAL) -- video mixing (Line of Fire)

    Y-Board
    -------
        Main Board (837-6565):
            315-5218       -- PCM sound controller
            315-5248 (x3)  -- hardware multiplier
            315-5249 (x3)  -- hardware divider
            315-5280 (PAL) -- Z80 address decoding
            315-5296       -- I/O chip
            315-5314 (PAL)
            315-5315 (PAL)
            315-5316 (PAL)
            315-5317 (PAL)
            315-5318 (PAL)
            315-5328 (PAL)

        Video Board (837-6566):
            315-5196       -- sprite generator
            315-5213 (PAL) -- sprite-related
            315-5242       -- color encoder
            315-5305       -- sprite generator
            315-5306 (x2)  -- video sync and rotation
            315-5312       -- video mixing
            315-5319 (PAL)
            315-5325 (PAL)


    Custom parts
    ------------
                   SYS1  SYS2  HANG  ENDU  PR16  S16A  S16B  SY18  SHNG  ORUN  XBRD  YBRD  SYSC  SY24  SY32
        315-5011:   xx    xx    xx    xx    xx    xx                                                         -- sprite line comparitor
        315-5012:   xx    xx    xx    xx    xx    xx                                                         -- sprite generator control
        315-5049:         xx    x2    x2    x2    x2                                                         -- tilemap generator
        315-5195:                                       xx          xx    xx                                 -- memory mapper
        315-5196:                                       xx          xx                xx                     -- sprite genereator
        315-5197:                                       xx          xx    xx    xx                           -- tilemap generator
        315-5211:                                                         xx                                 -- sprite generator
        315-5211A:                                                              xx                           -- sprite generator
        315-5218:                                                   xx    xx    xx    xx                     -- PCM sound controller
        315-5242:                                             xx    xx    xx    xx    xx    xx    xx    xx   -- color encoder
        315-5248:                                       xx                      x2    x3                     -- hardware multiplier
        315-5249:                                                               x2    x3                     -- hardware divider
        315-5250:                                       xx                      x2                           -- compare/timer
        315-5275:                                                               xx                           -- road generator
        315-5296:                                             xx                      xx    xx          xx   -- I/O chip
        315-5305:                                                                     xx                     --
        315-5312:                                                                     xx                     -- video mixing
        315-5313:                                             xx                            xx               -- VDP
        315-5360:                                             xx                                             -- memory mapper
        315-5361:                                             xx                                             -- sprite generator
        315-5362:                                             xx                                             -- tilemap generator
        315-5385:                                                                                       xx   -- ???
        315-5386:                                                                                       xx   -- tilemap generator
        315-5387:                                                                                       xx   -- sprite generator
        315-5388:                                                                                       xx   -- video mixing
        315-5436:                                             xx                                             -- sprite/tile banking
        315-5476:                                                                                       xx   -- ????

****************************************************************************

  Sega system16 and friends hardware

               CPU      Tiles      Sprites   Priority  Color     SCPU  Sound                Other
System C       68000    315-5313                       315-5242  z80   ym3438               315-5296(IO)
Space Harrier  68000x2                                 (c)       z80   ym2203 pcm(b)
System 16B     68000    315-5197   315-5196  GAL       (c)       z80   ym2151 upd7759       315-5195
After Burner   68000x2  315-5197   315-5211A GAL       315-5242  z80   ym2151 315-5218      315-5250(a) 315-5248(x2) 315-5249(x2) 315-5275(road)
System 18      68000    315-536x   315-536x            315-5242  z80   ym3834(x2) RF5c68(d) 315-3296(IO) 315-5313(vdp)
System 24      68000x2  315-5292   315-5293  315-5294  315-5242        ym2151 dac           315-5195(x3) 315-5296(IO)
Galaxy Force   68000x3             315-5296+ 315-5312  315-5242  z80   ym2151 315-5218      315-5296(IO)
System 32      V60      315-5386A  315-5387  315-5388  315-5242  z80   ym3834(x2) RF5c68(d) 315-5296(IO)

a) 315-5250: 68000 glue and address decoding

b) 8x8-bit voices entirely in TTL.  The 315-5218 is believed to be the
   integrated version of that

c) Resistor network and latches believed to be equivalent to the 315-5242

d) Also seen as 315-5476A and ASSP 5c105 and ASSP 5c68a

Quick review of the system16 hardware:

  Hang-on hardware:
    The first one.  Two tilemap planes, one sprite plane, one road
    plane.  The shadow capability doesn't seem to be used, the
    highlight/shadow switch in the 5242-equivalent is global for all
    colors.

  Space harrier hardware:
    Similar to hang-on, with per-color highlight/shadow selection, and
    the shadows are used.

  System16a / Pre-system16:
    Space harrier without the road generator.

  System16b:
    4-layer tilemap hardware in two pairs, with selection between each
    members on the pairs on a 8-lines basis.  Slightly better sprites.

  System18
    System 16b plus a genesis vdp.

  Outrun:
    System 16b tilemaps, frame buffered sprites with better zooming
    capabilities, and a road generator able to handle two roads
    simultaneously.

  Super hang-on:
    Outrun lite, with System 16b sprites instead of the frame buffered
    sprites, and only one of the two roads is actually used.

  X-Board:
    Outrun with a better fillrate and an even more flexible road
    generator.

  Y-Board:
    New design, with two sprite planes and no tilemaps.  The back
    sprite plane has a huge fillrate and the capability to have its
    frame buffer completely rotated.  Also, it has a palette
    indirection capability to allows for easier palette rotations.
    The front sprite plane is System 16b.

  System24:
    The odd one out.  Medium resolution.  Entirely ram-based, no
    graphics roms.  4-layer tilemap hardware in two pairs, selection
    on a 8-pixels basis.  Tile-based sprites(!) organised as a linked
    list.  The tilemap chip has been reused for model1 and model2,
    probably because they had it handy and it handles medium res.

  System32:
    5-layer tilemap hardware consisting of 4 independent rom-based
    layers with linescroll, lineselection, linezoom and window
    clipping capability and one simpler ram-based text plane.  Mixed
    ram/rom sprite engine with palette indirection, per-color priority
    (thankfully not actually used).  The sprite list includes jumping
    and clipping capabilities, and advanced hot-spot positioning.  The
    mixer chip adds totally dynamic priorities, alpha-blending of the
    tilemaps, per-component color control, and some other funnies we
    have not been able to decipher.

  ST-V (also know as Titan or the Saturn console):
    The ultimate 2D system.  Even more advanced tilemaps, with 6-dof
    roz support, alpha up to the wazoo and other niceties, known as
    the vdp2.  The sprite engine, vdp1, allows for any 4-point
    stretching of the sprites, actually giving polygonal 3D
    capabilities.  Interestingly, the mixer capabilities took a hit,
    with no real per-sprite mixer priority, which could be considered
    annoying for a 2D system.  It still allowed some beauties like
    Radiant Silvergun.

***************************************************************************/

#include "emu.h"
#include "segaic16.h"
#include "video/resnet.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define PRINT_UNUSUAL_MODES     (0)






const device_type SEGAIC16VID = &device_creator<segaic16_video_device>;

segaic16_video_device::segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGAIC16VID, "Sega 16-bit Video", tag, owner, clock, "segaic16_video", __FILE__),
		device_video_interface(mconfig, *this),
		m_display_enable(0),
		m_tileram(*this, "^tileram"),
		m_textram(*this, "^textram"),
		m_rotateram(*this, "^rotateram"),
		m_gfxdecode(*this)
{
	memset(m_rotate, 0, sizeof(m_rotate));
	memset(m_bg_tilemap, 0, sizeof(m_bg_tilemap));
	m_pagelatch_cb =  segaic16_video_pagelatch_delegate(FUNC(segaic16_video_device::tilemap_16b_fill_latch), this);
}

void segaic16_video_device::set_pagelatch_cb(device_t &device,segaic16_video_pagelatch_delegate newtilecb)
{
	segaic16_video_device &dev = downcast<segaic16_video_device &>(device);
	dev.m_pagelatch_cb = newtilecb;
}


//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void segaic16_video_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<segaic16_video_device &>(device).m_gfxdecode.set_tag(tag);
}


void segaic16_video_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	save_item(NAME(m_display_enable));

	m_pagelatch_cb.bind_relative_to(*owner());
}

void segaic16_video_device::device_reset()
{
}


/*************************************
 *
 *  Misc functions
 *
 *************************************/

void segaic16_video_device::set_display_enable(int enable)
{
	enable = (enable != 0);
	if (m_display_enable != enable)
	{
		m_screen->update_partial(m_screen->vpos());
		m_display_enable = enable;
	}
}



/*************************************
 *
 *  Draw a split tilemap in up to
 *  four pieces
 *
 *************************************/

void draw_virtual_tilemap(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 pages, UINT16 xscroll, UINT16 yscroll, UINT32 flags, UINT32 priority)
{
	int leftmin = -1, leftmax = -1, rightmin = -1, rightmax = -1;
	int topmin = -1, topmax = -1, bottommin = -1, bottommax = -1;
	rectangle pageclip;
	int page;


	if (info->flip)
	{
		pages = BITSWAP16(pages,
			3, 2, 1, 0,
			7, 6, 5, 4,
			11, 10, 9, 8,
			15, 14, 13, 12
			);

	}

	int width = screen.visible_area().max_x+1;
	int height = screen.visible_area().max_y+1;

	/* which half/halves of the virtual tilemap do we intersect in the X direction? */
	if (xscroll < 64*8 - width)
	{
		leftmin = 0;
		leftmax = width - 1;
		rightmin = -1;
	}
	else if (xscroll < 64*8)
	{
		leftmin = 0;
		leftmax = 64*8 - xscroll - 1;
		rightmin = leftmax + 1;
		rightmax = width - 1;
	}
	else if (xscroll < 128*8 - width)
	{
		rightmin = 0;
		rightmax = width - 1;
		leftmin = -1;
	}
	else
	{
		rightmin = 0;
		rightmax = 128*8 - xscroll - 1;
		leftmin = rightmax + 1;
		leftmax = width - 1;
	}

	/* which half/halves of the virtual tilemap do we intersect in the Y direction? */
	if (yscroll < 32*8 - height)
	{
		topmin = 0;
		topmax = height - 1;
		bottommin = -1;
	}
	else if (yscroll < 32*8)
	{
		topmin = 0;
		topmax = 32*8 - yscroll - 1;
		bottommin = topmax + 1;
		bottommax = height - 1;
	}
	else if (yscroll < 64*8 - height)
	{
		bottommin = 0;
		bottommax = height - 1;
		topmin = -1;
	}
	else
	{
		bottommin = 0;
		bottommax = 64*8 - yscroll - 1;
		topmin = bottommax + 1;
		topmax = height - 1;
	}

	// adjust split positions to compensate for flipping
	if (info->flip)
	{
		int temp;
		if (bottommin != -1) bottommin = height - 1 - bottommin;
		if (bottommax != -1) bottommax = height - 1 - bottommax;
		if (topmin != -1) topmin = height - 1 - topmin;
		if (topmax != -1) topmax = height - 1 - topmax;

		temp = bottommin;
		bottommin = topmax;
		topmax = temp;

		temp = bottommax;
		bottommax = topmin;
		topmin = temp;

		if (leftmin != -1) leftmin = width - 1 - leftmin;
		if (leftmax != -1) leftmax = width - 1 - leftmax;
		if (rightmin != -1) rightmin = width - 1 - rightmin;
		if (rightmax != -1) rightmax = width - 1 - rightmax;

		temp = leftmin;
		leftmin = rightmax;
		rightmax = temp;

		temp = leftmax;
		leftmax = rightmin;
		rightmin = temp;

	}

	/* draw the upper-left chunk */
	if (leftmin != -1 && topmin != -1)
	{
		pageclip.min_x = (leftmin < cliprect.min_x) ? cliprect.min_x : leftmin;
		pageclip.max_x = (leftmax > cliprect.max_x) ? cliprect.max_x : leftmax;
		pageclip.min_y = (topmin < cliprect.min_y) ? cliprect.min_y : topmin;
		pageclip.max_y = (topmax > cliprect.max_y) ? cliprect.max_y : topmax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 0) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(screen, bitmap, pageclip, flags, priority);
		}
	}

	/* draw the upper-right chunk */
	if (rightmin != -1 && topmin != -1)
	{
		pageclip.min_x = (rightmin < cliprect.min_x) ? cliprect.min_x : rightmin;
		pageclip.max_x = (rightmax > cliprect.max_x) ? cliprect.max_x : rightmax;
		pageclip.min_y = (topmin < cliprect.min_y) ? cliprect.min_y : topmin;
		pageclip.max_y = (topmax > cliprect.max_y) ? cliprect.max_y : topmax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 4) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(screen, bitmap, pageclip, flags, priority);
		}
	}

	/* draw the lower-left chunk */
	if (leftmin != -1 && bottommin != -1)
	{
		pageclip.min_x = (leftmin < cliprect.min_x) ? cliprect.min_x : leftmin;
		pageclip.max_x = (leftmax > cliprect.max_x) ? cliprect.max_x : leftmax;
		pageclip.min_y = (bottommin < cliprect.min_y) ? cliprect.min_y : bottommin;
		pageclip.max_y = (bottommax > cliprect.max_y) ? cliprect.max_y : bottommax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 8) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(screen, bitmap, pageclip, flags, priority);
		}
	}

	/* draw the lower-right chunk */
	if (rightmin != -1 && bottommin != -1)
	{
		pageclip.min_x = (rightmin < cliprect.min_x) ? cliprect.min_x : rightmin;
		pageclip.max_x = (rightmax > cliprect.max_x) ? cliprect.max_x : rightmax;
		pageclip.min_y = (bottommin < cliprect.min_y) ? cliprect.min_y : bottommin;
		pageclip.max_y = (bottommax > cliprect.max_y) ? cliprect.max_y : bottommax;
		if (pageclip.min_x <= pageclip.max_x && pageclip.min_y <= pageclip.max_y)
		{
			page = (pages >> 12) & 0xf;
			info->tilemaps[page]->set_scrollx(0, xscroll);
			info->tilemaps[page]->set_scrolly(0, yscroll);
			info->tilemaps[page]->draw(screen, bitmap, pageclip, flags, priority);
		}
	}
}



/*******************************************************************************************
 *
 *  Hang On/System 16A-style tilemaps
 *
 *  4 total pages (Hang On)
 *  8 total pages (System 16A)
 *  Column/rowscroll enabled via external signals
 *
 *  Tile format:
 *      Bits               Usage
 *      ??------ --------  Unknown
 *      --b----- --------  Tile bank select
 *      ---p---- --------  Tile priority versus sprites
 *      ----cccc ccc-----  Tile color palette
 *      ----nnnn nnnnnnnn  Tile index
 *
 *  Text format:
 *      Bits               Usage
 *      ????---- --------  Unknown
 *      ----p--- --------  Priority
 *      -----ccc --------  Tile color palette
 *      -------- nnnnnnnn  Tile index
 *
 *  Text RAM:
 *      Offset   Bits               Usage
 *      E8C      -aaa-bbb -ccc-ddd  Background tilemap page select (screen flipped)
 *      E8E      -aaa-bbb -ccc-ddd  Foreground tilemap page select (screen flipped)
 *      E9C      -aaa-bbb -ccc-ddd  Background tilemap page select
 *      E9E      -aaa-bbb -ccc-ddd  Foreground tilemap page select
 *      F24      -------- vvvvvvvv  Foreground tilemap vertical scroll
 *      F26      -------- vvvvvvvv  Background tilemap vertical scroll
 *      F30-F7D  -------- vvvvvvvv  Foreground tilemap per-16-pixel-column vertical scroll (every 2 words)
 *      F32-F7F  -------- vvvvvvvv  Background tilemap per-16-pixel-column vertical scroll (every 2 words)
 *      F80-FED  -------h hhhhhhhh  Foreground tilemap per-8-pixel-row horizontal scroll (every 2 words)
 *      F82-FEF  -------h hhhhhhhh  Background tilemap per-8-pixel-row horizontal scroll (every 2 words)
 *      FF8      -------h hhhhhhhh  Foreground tilemap horizontal scroll
 *      FFA      -------h hhhhhhhh  Background tilemap horizontal scroll
 *
 *******************************************************************************************/

TILE_GET_INFO_MEMBER( segaic16_video_device::tilemap_16a_tile_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)tilemap.user_data();
	UINT16 data = info->rambase[tile_index];
	int code = ((data >> 1) & 0x1000) | (data & 0xfff);
	int color = (data >> 5) & 0x7f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
	tileinfo.category = (data >> 12) & 1;
}


TILE_GET_INFO_MEMBER( segaic16_video_device::tilemap_16a_text_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)tilemap.user_data();
	UINT16 data = info->rambase[tile_index];
	int color = (data >> 8) & 0x07;
	int code = data & 0xff;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
	tileinfo.category = (data >> 11) & 1;
}


void tilemap_16a_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority)
{
	UINT16 *textram = info->textram;

	/* note that the scrolling for these games can only scroll as much as the top-left */
	/* page; in order to scroll beyond that they swap pages and reset the scroll value */
	UINT16 xscroll = textram[0xff8/2 + which] & 0x1ff;
	UINT16 yscroll = textram[0xf24/2 + which] & 0x0ff;
	UINT16 pages = textram[(info->flip ? 0xe8e/2 : 0xe9e/2) - which];
	int x, y;

	/* pages are swapped along the X direction, and there are only 8 of them */
	pages = ((pages >> 4) & 0x0707) | ((pages << 4) & 0x7070);
	if (info->numpages == 4)
		pages &= 0x3333;

	/* column AND row scroll */
	if (info->colscroll && info->rowscroll)
	{
		if (PRINT_UNUSUAL_MODES) osd_printf_debug("Column AND row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowcolclip;

			/* adjust to clip this row only */
			rowcolclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowcolclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* loop over column chunks */
			for (x = cliprect.min_x & ~15; x <= cliprect.max_x; x += 16)
			{
				UINT16 effxscroll, effyscroll;

				/* adjust to clip this column only */
				rowcolclip.min_x = (x < cliprect.min_x) ? cliprect.min_x : x;
				rowcolclip.max_x = (x + 15 > cliprect.max_x) ? cliprect.max_x : x + 15;

				/* get the effective scroll values */
				effxscroll = textram[0xf80/2 + rowscrollindex * 2 + which] & 0x1ff;
				effyscroll = textram[0xf30/2 + (x/16) * 2 + which] & 0x0ff;

				/* adjust the xscroll for flipped screen */
				if (info->flip)
					effxscroll += 17;

				/* draw the chunk */
				effxscroll = (0xc8 - effxscroll + info->xoffs) & 0x3ff;
				effyscroll = effyscroll & 0x1ff;
				draw_virtual_tilemap(screen, info, bitmap, rowcolclip, pages, effxscroll, effyscroll, flags, priority);
			}
		}
	}
	else if (info->colscroll)
	{
		if (PRINT_UNUSUAL_MODES) osd_printf_debug("Column scroll\n");

		/* loop over column chunks */
		for (x = cliprect.min_x & ~15; x <= cliprect.max_x; x += 16)
		{
			rectangle colclip = cliprect;
			UINT16 effxscroll, effyscroll;

			/* adjust to clip this row only */
			colclip.min_x = (x < cliprect.min_x) ? cliprect.min_x : x;
			colclip.max_x = (x + 15 > cliprect.max_x) ? cliprect.max_x : x + 15;

			/* get the effective scroll values */
			effxscroll = xscroll;
			effyscroll = textram[0xf30/2 + (x/16) * 2 + which] & 0x0ff;

			/* adjust the xscroll for flipped screen */
			if (info->flip)
				effxscroll += 17;

			/* draw the chunk */
			effxscroll = (0xc8 - effxscroll + info->xoffs) & 0x3ff;
			effyscroll = effyscroll & 0x1ff;
			draw_virtual_tilemap(screen, info, bitmap, colclip, pages, effxscroll, effyscroll, flags, priority);
		}
	}
	else if (info->rowscroll)
	{
		if (PRINT_UNUSUAL_MODES) osd_printf_debug("Row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowclip = cliprect;
			UINT16 effxscroll, effyscroll;

			/* adjust to clip this row only */
			rowclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* get the effective scroll values */
			effxscroll = textram[0xf80/2 + rowscrollindex * 2 + which] & 0x1ff;
			effyscroll = yscroll;

			/* adjust the xscroll for flipped screen */
			if (info->flip)
				effxscroll += 17;

			/* draw the chunk */
			effxscroll = (0xc8 - effxscroll + info->xoffs) & 0x3ff;
			effyscroll = effyscroll & 0x1ff;
			draw_virtual_tilemap(screen, info, bitmap, rowclip, pages, effxscroll, effyscroll, flags, priority);
		}
	}
	else
	{
		/* adjust the xscroll for flipped screen */
		if (info->flip)
			xscroll += 17;
		xscroll = (0xc8 - xscroll + info->xoffs) & 0x3ff;
		yscroll = yscroll & 0x1ff;
		draw_virtual_tilemap(screen, info, bitmap, cliprect, pages, xscroll, yscroll, flags, priority);
	}
}



/*******************************************************************************************
 *
 *  System 16B-style tilemaps
 *
 *  16 total pages
 *  Column/rowscroll enabled via bits in text layer
 *  Alternate tilemap support
 *
 *  Tile format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -??----- --------  Unknown
 *      ---ccccc cc------  Tile color palette
 *      ---nnnnn nnnnnnnn  Tile index
 *
 *  Text format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -???---- --------  Unknown
 *      ----ccc- --------  Tile color palette
 *      -------n nnnnnnnn  Tile index
 *
 *  Alternate tile format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -??----- --------  Unknown
 *      ----cccc ccc-----  Tile color palette
 *      ---nnnnn nnnnnnnn  Tile index
 *
 *  Alternate text format:
 *      Bits               Usage
 *      p------- --------  Tile priority versus sprites
 *      -???---- --------  Unknown
 *      -----ccc --------  Tile color palette
 *      -------- nnnnnnnn  Tile index
 *
 *  Text RAM:
 *      Offset   Bits               Usage
 *      E80      aaaabbbb ccccdddd  Foreground tilemap page select
 *      E82      aaaabbbb ccccdddd  Background tilemap page select
 *      E84      aaaabbbb ccccdddd  Alternate foreground tilemap page select
 *      E86      aaaabbbb ccccdddd  Alternate background tilemap page select
 *      E90      c------- --------  Foreground tilemap column scroll enable
 *               -------v vvvvvvvv  Foreground tilemap vertical scroll
 *      E92      c------- --------  Background tilemap column scroll enable
 *               -------v vvvvvvvv  Background tilemap vertical scroll
 *      E94      -------v vvvvvvvv  Alternate foreground tilemap vertical scroll
 *      E96      -------v vvvvvvvv  Alternate background tilemap vertical scroll
 *      E98      r------- --------  Foreground tilemap row scroll enable
 *               ------hh hhhhhhhh  Foreground tilemap horizontal scroll
 *      E9A      r------- --------  Background tilemap row scroll enable
 *               ------hh hhhhhhhh  Background tilemap horizontal scroll
 *      E9C      ------hh hhhhhhhh  Alternate foreground tilemap horizontal scroll
 *      E9E      ------hh hhhhhhhh  Alternate background tilemap horizontal scroll
 *      F16-F3F  -------- vvvvvvvv  Foreground tilemap per-16-pixel-column vertical scroll
 *      F56-F7F  -------- vvvvvvvv  Background tilemap per-16-pixel-column vertical scroll
 *      F80-FB7  a------- --------  Foreground tilemap per-8-pixel-row alternate tilemap enable
 *               -------h hhhhhhhh  Foreground tilemap per-8-pixel-row horizontal scroll
 *      FC0-FF7  a------- --------  Background tilemap per-8-pixel-row alternate tilemap enable
 *               -------h hhhhhhhh  Background tilemap per-8-pixel-row horizontal scroll
 *
 *******************************************************************************************/

TILE_GET_INFO_MEMBER( segaic16_video_device::tilemap_16b_tile_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)tilemap.user_data();
	UINT16 data = info->rambase[tile_index];
	int color = (data >> 6) & 0x7f;
	int code = data & 0x1fff;

	code = info->bank[code / info->banksize] * info->banksize + code % info->banksize;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


TILE_GET_INFO_MEMBER( segaic16_video_device::tilemap_16b_text_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)tilemap.user_data();
	UINT16 data = info->rambase[tile_index];
	int bank = info->bank[0];
	int color = (data >> 9) & 0x07;
	int code = data & 0x1ff;

	SET_TILE_INFO_MEMBER(0, bank * info->banksize + code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


TILE_GET_INFO_MEMBER( segaic16_video_device::tilemap_16b_alt_tile_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)tilemap.user_data();
	UINT16 data = info->rambase[tile_index];
	int color = (data >> 5) & 0x7f;
	int code = data & 0x1fff;

	code = info->bank[code / info->banksize] * info->banksize + code % info->banksize;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


TILE_GET_INFO_MEMBER( segaic16_video_device::tilemap_16b_alt_text_info )
{
	const struct tilemap_callback_info *info = (const struct tilemap_callback_info *)tilemap.user_data();
	UINT16 data = info->rambase[tile_index];
	int bank = info->bank[0];
	int color = (data >> 8) & 0x07;
	int code = data & 0xff;

	SET_TILE_INFO_MEMBER(0, bank * info->banksize + code, color, 0);
	tileinfo.category = (data >> 15) & 1;
}


void tilemap_16b_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority)
{
	UINT16 *textram = info->textram;
	UINT16 xscroll, yscroll, pages;
	int x, y;

	/* get global values */
	xscroll = info->latched_xscroll[which];
	yscroll = info->latched_yscroll[which];
	pages = info->latched_pageselect[which];

	/* column scroll? */
	if (yscroll & 0x8000)
	{
		if (PRINT_UNUSUAL_MODES) osd_printf_debug("Column AND row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowcolclip;

			/* adjust to clip this row only */
			rowcolclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowcolclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* loop over column chunks */
			for (x = ((cliprect.min_x + 8) & ~15) - 8; x <= cliprect.max_x; x += 16)
			{
				UINT16 effxscroll, effyscroll, rowscroll;
				UINT16 effpages = pages;

				/* adjust to clip this column only */
				rowcolclip.min_x = (x < cliprect.min_x) ? cliprect.min_x : x;
				rowcolclip.max_x = (x + 15 > cliprect.max_x) ? cliprect.max_x : x + 15;

				/* get the effective scroll values */
				rowscroll = textram[0xf80/2 + 0x40/2 * which + rowscrollindex];
				effxscroll = (xscroll & 0x8000) ? rowscroll : xscroll;
				effyscroll = textram[0xf16/2 + 0x40/2 * which + (x+8)/16];

				/* are we using an alternate? */
				if (rowscroll & 0x8000)
				{
					effxscroll = info->latched_xscroll[which + 2];
					effyscroll = info->latched_yscroll[which + 2];
					effpages = info->latched_pageselect[which + 2];
				}

				/* draw the chunk */
				effxscroll = (0xc0 - effxscroll + info->xoffs) & 0x3ff;
				effyscroll = effyscroll & 0x1ff;
				draw_virtual_tilemap(screen, info, bitmap, rowcolclip, effpages, effxscroll, effyscroll, flags, priority);
			}
		}
	}
	else
	{
		if (PRINT_UNUSUAL_MODES) osd_printf_debug("Row scroll\n");

		/* loop over row chunks */
		for (y = cliprect.min_y & ~7; y <= cliprect.max_y; y += 8)
		{
			int rowscrollindex = (info->flip ? (216 - y) : y) / 8;
			rectangle rowclip = cliprect;
			UINT16 effxscroll, effyscroll, rowscroll;
			UINT16 effpages = pages;

			/* adjust to clip this row only */
			rowclip.min_y = (y < cliprect.min_y) ? cliprect.min_y : y;
			rowclip.max_y = (y + 7 > cliprect.max_y) ? cliprect.max_y : y + 7;

			/* get the effective scroll values */
			rowscroll = textram[0xf80/2 + 0x40/2 * which + rowscrollindex];
			effxscroll = (xscroll & 0x8000) ? rowscroll : xscroll;
			effyscroll = yscroll;

			/* are we using an alternate? */
			if (rowscroll & 0x8000)
			{
				effxscroll = info->latched_xscroll[which + 2];
				effyscroll = info->latched_yscroll[which + 2];
				effpages = info->latched_pageselect[which + 2];
			}

			/* draw the chunk */
			effxscroll = (0xc0 - effxscroll + info->xoffs) & 0x3ff;
			effyscroll = effyscroll & 0x1ff;
			draw_virtual_tilemap(screen, info, bitmap, rowclip, effpages, effxscroll, effyscroll, flags, priority);
		}
	}
}




void segaic16_video_device::tilemap_16b_fill_latch(int i, UINT16* latched_pageselect, UINT16* latched_yscroll, UINT16* latched_xscroll, UINT16* textram)
{
	latched_pageselect[i] = textram[0xe80 / 2 + i];
	latched_yscroll[i] = textram[0xe90/2 + i];
	latched_xscroll[i] = textram[0xe98/2 + i];
//  printf("%02x returning latched page select %04x scrollx %04x scrolly %04x\n", i, latched_pageselect[i], latched_xscroll[i], latched_yscroll[i]);
}

TIMER_CALLBACK_MEMBER( segaic16_video_device::tilemap_16b_latch_values )
{
	struct tilemap_info *info = &m_bg_tilemap[param];
	UINT16 *textram = info->textram;
	int i;

	/* latch the scroll and page select values */
	for (i = 0; i < 4; i++)
	{
		m_pagelatch_cb(i, info->latched_pageselect, info->latched_yscroll, info->latched_xscroll, textram);
	}

	/* set a timer to do this again next frame */
	info->latch_timer->adjust(m_screen->time_until_pos(261), param);
}


void tilemap_16b_reset(screen_device &screen, struct tilemap_info *info)
{
	/* set a timer to latch values on scanline 261 */
	info->latch_timer->adjust(screen.time_until_pos(261), info->index);
}



/*************************************
 *
 *  General tilemap initialization
 *
 *************************************/

void segaic16_video_device::tilemap_init(int which, int type, int colorbase, int xoffs, int numbanks)
{
	struct tilemap_info *info = &m_bg_tilemap[which];
	tilemap_get_info_delegate get_text_info;
	tilemap_get_info_delegate get_tile_info;
	int pagenum;
	int i;

	/* reset the tilemap info */
	memset(info, 0, sizeof(*info));
	info->index = which;
	info->type = type;
	for (i = 0; i < numbanks; i++)
		info->bank[i] = i;
	info->banksize = 0x2000 / numbanks;
	info->xoffs = xoffs;

	/* set up based on which tilemap */
	switch (which)
	{
		case 0:
			info->textram = m_textram;
			info->tileram = m_tileram;
			break;

		default:
			fatalerror("Invalid tilemap index specified in tilemap_init\n");
	}

	/* determine the parameters of the tilemaps */
	switch (type)
	{
		case SEGAIC16_TILEMAP_HANGON:
			get_text_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16a_text_info),this);
			get_tile_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16a_tile_info),this);
			info->numpages = 4;
			info->draw_layer = tilemap_16a_draw_layer;
			info->reset = nullptr;
			info->latch_timer = nullptr;
			break;

		case SEGAIC16_TILEMAP_16A:
			get_text_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16a_text_info),this);
			get_tile_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16a_tile_info),this);
			info->numpages = 8;
			info->draw_layer = tilemap_16a_draw_layer;
			info->reset = nullptr;
			info->latch_timer = nullptr;
			break;

		case SEGAIC16_TILEMAP_16B:
			get_text_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16b_text_info),this);
			get_tile_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16b_tile_info),this);
			info->numpages = 16;
			info->draw_layer = tilemap_16b_draw_layer;
			info->reset = tilemap_16b_reset;
			info->latch_timer = machine().scheduler().timer_alloc( timer_expired_delegate(FUNC(segaic16_video_device::tilemap_16b_latch_values),this) );
			break;

		case SEGAIC16_TILEMAP_16B_ALT:
			get_text_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16b_alt_text_info),this);
			get_tile_info = tilemap_get_info_delegate(FUNC(segaic16_video_device::tilemap_16b_alt_tile_info),this);
			info->numpages = 16;
			info->draw_layer = tilemap_16b_draw_layer;
			info->reset = tilemap_16b_reset;
			info->latch_timer = machine().scheduler().timer_alloc( timer_expired_delegate(FUNC(segaic16_video_device::tilemap_16b_latch_values),this) );
			break;

		default:
			fatalerror("Invalid tilemap type specified in tilemap_init\n");
	}

	/* create the tilemap for the text layer */
	info->textmap = &machine().tilemap().create(m_gfxdecode, get_text_info, TILEMAP_SCAN_ROWS,  8,8, 64,28);

	/* configure it */
	info->textmap_info.rambase = info->textram;
	info->textmap_info.bank = info->bank;
	info->textmap_info.banksize = info->banksize;
	info->textmap->set_user_data(&info->textmap_info);
	info->textmap->set_palette_offset(colorbase);
	info->textmap->set_transparent_pen(0);
	info->textmap->set_scrolldx(-192 + xoffs, -192 + xoffs);
	info->textmap->set_scrolldy(0, 0);

	/* create the tilemaps for the tile pages */
	for (pagenum = 0; pagenum < info->numpages; pagenum++)
	{
		/* each page is 64x32 */
		info->tilemaps[pagenum] = &machine().tilemap().create(m_gfxdecode, get_tile_info, TILEMAP_SCAN_ROWS,  8,8, 64,32);

		/* configure the tilemap */
		info->tmap_info[pagenum].rambase = info->tileram + pagenum * 64*32;
		info->tmap_info[pagenum].bank = info->bank;
		info->tmap_info[pagenum].banksize = info->banksize;
		info->tilemaps[pagenum]->set_user_data(&info->tmap_info[pagenum]);
		info->tilemaps[pagenum]->set_palette_offset(colorbase);
		info->tilemaps[pagenum]->set_transparent_pen(0);
		info->tilemaps[pagenum]->set_scrolldx(0, 0);
		info->tilemaps[pagenum]->set_scrolldy(0, 0);
	}

	save_item(NAME(info->flip), which);
	save_item(NAME(info->rowscroll), which);
	save_item(NAME(info->colscroll), which);
	save_item(NAME(info->bank), which);
	save_item(NAME(info->latched_xscroll), which);
	save_item(NAME(info->latched_yscroll), which);
	save_item(NAME(info->latched_pageselect), which);
}



/*************************************
 *
 *  General tilemap rendering
 *
 *************************************/

void segaic16_video_device::tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int map, int priority, int priority_mark)
{
	struct tilemap_info *info = &m_bg_tilemap[which];

	/* text layer is a special common case */
	if (map == SEGAIC16_TILEMAP_TEXT)
		info->textmap->draw(screen, bitmap, cliprect, priority, priority_mark);

	/* other layers are handled differently per-system */
	else
		(*info->draw_layer)(screen, info, bitmap, cliprect, map, priority, priority_mark);
}



/*************************************
 *
 *  General tilemap reset
 *
 *************************************/

void segaic16_video_device::tilemap_reset(screen_device &screen)
{
	struct tilemap_info *info = &m_bg_tilemap[0];

	if (info->reset)
		(*info->reset)(screen, info);
}



/*************************************
 *
 *  General tilemap banking
 *
 *************************************/

void segaic16_video_device::tilemap_set_bank(int which, int banknum, int offset)
{
	struct tilemap_info *info = &m_bg_tilemap[which];

	if (info->bank[banknum] != offset)
	{
		m_screen->update_partial(m_screen->vpos());
		info->bank[banknum] = offset;
		machine().tilemap().mark_all_dirty();
	}
}



/*************************************
 *
 *  General tilemap screen flipping
 *
 *************************************/

void segaic16_video_device::tilemap_set_flip(int which, int flip)
{
	struct tilemap_info *info = &m_bg_tilemap[which];
	int pagenum;

	flip = (flip != 0);
	if (info->flip != flip)
	{
		m_screen->update_partial(m_screen->vpos());
		info->flip = flip;
		info->textmap->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
		for (pagenum = 0; pagenum < info->numpages; pagenum++)
			info->tilemaps[pagenum]->set_flip(flip ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}
}



/*************************************
 *
 *  General tilemap row scroll enable
 *
 *************************************/

void segaic16_video_device::tilemap_set_rowscroll(int which, int enable)
{
	struct tilemap_info *info = &m_bg_tilemap[which];

	enable = (enable != 0);
	if (info->rowscroll != enable)
	{
		m_screen->update_partial(m_screen->vpos());
		info->rowscroll = enable;
	}
}



/*************************************
 *
 *  General tilemap column scroll enable
 *
 *************************************/

void segaic16_video_device::tilemap_set_colscroll(int which, int enable)
{
	struct tilemap_info *info = &m_bg_tilemap[which];

	enable = (enable != 0);
	if (info->colscroll != enable)
	{
		m_screen->update_partial(m_screen->vpos());
		info->colscroll = enable;
	}
}



/*************************************
 *
 *  General tilemap write handlers
 *
 *************************************/

READ16_MEMBER( segaic16_video_device::tileram_r )
{
	return m_tileram[offset];
}


WRITE16_MEMBER( segaic16_video_device::tileram_w )
{
	COMBINE_DATA(&m_tileram[offset]);
	m_bg_tilemap[0].tilemaps[offset / (64*32)]->mark_tile_dirty(offset % (64*32));
}


READ16_MEMBER( segaic16_video_device::textram_r )
{
	return m_textram[offset];
}


WRITE16_MEMBER( segaic16_video_device::textram_w )
{
	/* certain ranges need immediate updates */
	if (offset >= 0xe80/2)
		m_screen->update_partial(m_screen->vpos());

	COMBINE_DATA(&m_textram[offset]);
	m_bg_tilemap[0].textmap->mark_tile_dirty(offset);
}







/*************************************
 *
 *  General rotation initialization
 *
 *************************************/

void segaic16_video_device::rotate_init(int which, int type, int colorbase)
{
	struct rotate_info *info = &m_rotate[which];

	/* reset the tilemap info */
	memset(info, 0, sizeof(*info));
	info->index = which;
	info->type = type;
	info->colorbase = colorbase;

	/* set up based on which road generator */
	switch (which)
	{
		case 0:
			info->rotateram = m_rotateram;
			break;

		default:
			fatalerror("Invalid rotate index specified in rotate_init\n");
	}

	/* determine the parameters of the rotate */
	switch (type)
	{
		case SEGAIC16_ROTATE_YBOARD:
			info->ramsize = 0x800;
			break;

		default:
			fatalerror("Invalid rotate system specified in rotate_init\n");
	}

	/* allocate a buffer for swapping */
	info->buffer = std::make_unique<UINT16[]>(info->ramsize/2);

	save_item(NAME(info->colorbase), which);
	save_pointer(NAME((UINT8 *) info->buffer.get()), info->ramsize, which);
}



/*************************************
 *
 *  General rotation drawing
 *
 *************************************/

void segaic16_video_device::rotate_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, bitmap_ind16 &srcbitmap)
{
	struct rotate_info *info = &m_rotate[which];
	INT32 currx = (info->buffer[0x3f0] << 16) | info->buffer[0x3f1];
	INT32 curry = (info->buffer[0x3f2] << 16) | info->buffer[0x3f3];
	INT32 dyy = (info->buffer[0x3f4] << 16) | info->buffer[0x3f5];
	INT32 dxx = (info->buffer[0x3f6] << 16) | info->buffer[0x3f7];
	INT32 dxy = (info->buffer[0x3f8] << 16) | info->buffer[0x3f9];
	INT32 dyx = (info->buffer[0x3fa] << 16) | info->buffer[0x3fb];
	int x, y;

	/* advance forward based on the clip rect */
	currx += dxx * (cliprect.min_x + 27) + dxy * cliprect.min_y;
	curry += dyx * (cliprect.min_x + 27) + dyy * cliprect.min_y;

	/* loop over screen Y coordinates */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dest = &bitmap.pix16(y);
		UINT16 *src = &srcbitmap.pix16(0);
		UINT8 *pri = &priority_bitmap.pix8(y);
		INT32 tx = currx;
		INT32 ty = curry;

		/* loop over screen X coordinates */
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			/* fetch the pixel from the source bitmap */
			int sx = (tx >> 14) & 0x1ff;
			int sy = (ty >> 14) & 0x1ff;
			int pix = src[sy * srcbitmap.rowpixels() + sx];

			/* non-zero pixels get written; everything else is the scanline color */
			if (pix != 0xffff)
			{
				*dest++ = (pix & 0x1ff) | ((pix >> 6) & 0x200) | ((pix >> 3) & 0xc00) | 0x1000;
				*pri++ = (pix >> 8) | 1;
			}
			else
			{
				*dest++ = info->colorbase + sy;
				*pri++ = 0xff;
			}

			/* advance the source X/Y pointers */
			tx += dxx;
			ty += dyx;
		}

		/* advance the source X/Y pointers */
		currx += dxy;
		curry += dyy;
	}
}



/*************************************
 *
 *  General road control read/write
 *
 *************************************/

READ16_MEMBER( segaic16_video_device::rotate_control_r )
{
	struct rotate_info *info = &m_rotate[0];

	if (info->buffer)
	{
		UINT32 *src = (UINT32 *)info->rotateram;
		UINT32 *dst = (UINT32 *)info->buffer.get();
		int i;

		/* swap the halves of the rotation RAM */
		for (i = 0; i < info->ramsize/4; i++)
		{
			UINT32 temp = *src;
			*src++ = *dst;
			*dst++ = temp;
		}
	}

	return 0xffff;
}
