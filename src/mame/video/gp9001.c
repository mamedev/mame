/* GP9001 Video Controller */

/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use one or more Toaplan L7A0498 GP9001 graphic controllers.

  The simpler hardware of these games use one GP9001 controller.

  Next we have games that use two GP9001 controllers, the mixing of
  the VDPs depends on a PAL on the motherboard.
  (mixing handled in toaplan2.c)

  Finally we have games using one GP9001 controller and an additional
  text tile layer, which has highest priority. This text tile layer
  appears to have line-scroll support. Some of these games copy the
  text tile gfx data to RAM from the main CPU ROM, which easily allows
  for effects to be added to the tiles, by manipulating the text tile
  gfx data. The tiles are then dynamically decoded from RAM before
  displaying them.


 To Do / Unknowns
    -  What do Scroll registers 0Eh and 0Fh really do ????
    -  Snow Bros 2 sets bit 6 of the sprite X info word during weather
        world map, and bits 4, 5 and 6 of the sprite X info word during
        the Rabbit boss screen - reasons are unknown.
    -  Fourth set of scroll registers have been used for Sprite scroll
        though it may not be correct. For most parts this looks right
        except for Snow Bros 2 when in the rabbit boss screen (all sprites
        jump when big green nasty (which is the foreground layer) comes
        in from the left)
    -  Teki Paki tests video RAM from address 0 past SpriteRAM to $37ff.
        This seems to be a bug in Teki Paki's vram test routine !




 GP9001 Tile RAM format (each tile takes up 32 bits)

  0         1         2         3
  ---- ---- ---- ---- xxxx xxxx xxxx xxxx = Tile number (0 - FFFFh)
  ---- ---- -xxx xxxx ---- ---- ---- ---- = Color (0 - 7Fh)
  ---- ---- ?--- ---- ---- ---- ---- ---- = unknown / unused
  ---- xxxx ---- ---- ---- ---- ---- ---- = Priority (0 - Fh)
  ???? ---- ---- ---- ---- ---- ---- ---- = unknown / unused / possible flips

Sprites are of varying sizes between 8x8 and 128x128 with any variation
in between, in multiples of 8 either way.

Here we draw the first 8x8 part of the sprite, then by using the sprite
dimensions, we draw the rest of the 8x8 parts to produce the complete
sprite.

There seems to be sprite buffering - double buffering actually.

 GP9001 Sprite RAM format (data for each sprite takes up 4 words)

  0
  ---- ----  ---- --xx = top 2 bits of Sprite number
  ---- ----  xxxx xx-- = Color (0 - 3Fh)
  ---- xxxx  ---- ---- = Priority (0 - Fh)
  ---x ----  ---- ---- = Flip X
  --x- ----  ---- ---- = Flip Y
  -x-- ----  ---- ---- = Multi-sprite
  x--- ----  ---- ---- = Show sprite ?

  1
  xxxx xxxx  xxxx xxxx = Sprite number (top two bits in word 0)

  2
  ---- ----  ---- xxxx = Sprite X size (add 1, then multiply by 8)
  ---- ----  -??? ---- = unknown - used in Snow Bros. 2
  xxxx xxxx  x--- ---- = X position

  3
  ---- ----  ---- xxxx = Sprite Y size (add 1, then multiply by 8)
  ---- ----  -??? ---- = unknown / unused
  xxxx xxxx  x--- ---- = Y position





 GP9001 Scroll Registers (hex) :

    00      Background scroll X (X flip off)
    01      Background scroll Y (Y flip off)
    02      Foreground scroll X (X flip off)
    03      Foreground scroll Y (Y flip off)
    04      Top (text) scroll X (X flip off)
    05      Top (text) scroll Y (Y flip off)
    06      Sprites    scroll X (X flip off) ???
    07      Sprites    scroll Y (Y flip off) ???
    0E      ??? Initialise Video controller at startup ???
    0F      Scroll update complete ??? (Not used in Ghox and V-Five)

    80      Background scroll X (X flip on)
    81      Background scroll Y (Y flip on)
    82      Foreground scroll X (X flip on)
    83      Foreground scroll Y (Y flip on)
    84      Top (text) scroll X (X flip on)
    85      Top (text) scroll Y (Y flip on)
    86      Sprites    scroll X (X flip on) ???
    87      Sprites    scroll Y (Y flip on) ???
    8F      Same as 0Fh except flip bit is active


Scroll Register 0E writes (Video controller inits ?) from different games:

Teki-Paki        | Ghox             | Knuckle Bash     | Truxton 2        |
0003, 0002, 4000 | ????, ????, ???? | 0202, 0203, 4200 | 0003, 0002, 4000 |

Dogyuun          | Batsugun         |
0202, 0203, 4200 | 0202, 0203, 4200 |
1202, 1203, 5200 | 1202, 1203, 5200 | <--- Second video controller

Pipi & Bibis     | Fix Eight        | V-Five           | Snow Bros. 2     |
0003, 0002, 4000 | 0202, 0203, 4200 | 0202, 0203, 4200 | 0202, 0203, 4200 |

***************************************************************************/


#include "emu.h"
#include "gp9001.h"

/*
 Single VDP mixing priority note:

 Initial thoughts were that 16 levels of priority exist for both sprites and tilemaps, ie GP9001_PRIMASK 0xf
 However the end of level scene rendered on the first VDP in Batsugun strongly suggests otherwise.

 Sprites  have 'priority' bits of 0x0600 (level 0x6) set
 Tilemaps have 'priority' bits of 0x7000 (level 0x7) set

 If a mask of 0xf is used then the tilemaps render above the sprites, which causes the V bonus items near the
 counters to be invisible (in addition to the English character quote text)

 using a mask of 0xe causes both priority levels to be equal, allowing the sprites to render above the tilemap.

 The alternative option of allowing sprites to render a priority level higher than tilemaps breaks at least the
 'Welcome to..' screen in Batrider after selecting your character.

 It is unknown if the current solution breaks anything.  The majority of titles don't make extensive use of the
 priority system.

*/
#define GP9001_PRIMASK (0x000e)


static WRITE16_DEVICE_HANDLER( gp9001_bg_tmap_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->bg.vram16[offset]);
	vdp->bg.tmap->mark_tile_dirty(offset/2);
}

static WRITE16_DEVICE_HANDLER( gp9001_fg_tmap_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->fg.vram16[offset]);
	vdp->fg.tmap->mark_tile_dirty(offset/2);
}

static WRITE16_DEVICE_HANDLER( gp9001_top_tmap_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->top.vram16[offset]);
	vdp->top.tmap->mark_tile_dirty(offset/2);
}

static READ16_DEVICE_HANDLER( gp9001_bg_tmap_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->bg.vram16[offset];
}

static READ16_DEVICE_HANDLER( gp9001_fg_tmap_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->fg.vram16[offset];
}

static READ16_DEVICE_HANDLER( gp9001_top_tmap_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->top.vram16[offset];
}

static READ16_DEVICE_HANDLER( gp9001_spram_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->sp.vram16[offset];
}

static WRITE16_DEVICE_HANDLER( gp9001_spram_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->sp.vram16[offset]);
}

static ADDRESS_MAP_START( gp9001vdp_map, AS_0, 16, gp9001vdp_device )
	AM_RANGE(0x0000, 0x0fff) AM_DEVREADWRITE_LEGACY(DEVICE_SELF, gp9001_bg_tmap_r, gp9001_bg_tmap_w)
	AM_RANGE(0x1000, 0x1fff) AM_DEVREADWRITE_LEGACY(DEVICE_SELF, gp9001_fg_tmap_r, gp9001_fg_tmap_w)
	AM_RANGE(0x2000, 0x2fff) AM_DEVREADWRITE_LEGACY(DEVICE_SELF, gp9001_top_tmap_r, gp9001_top_tmap_w)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREADWRITE_LEGACY(DEVICE_SELF, gp9001_spram_r, gp9001_spram_w)
	AM_RANGE(0x3800, 0x3fff) AM_RAM // sprite mirror?
ADDRESS_MAP_END


const device_type GP9001_VDP = &device_creator<gp9001vdp_device>;

gp9001vdp_device::gp9001vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GP9001_VDP, "GP9001_VDP", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_space_config("gp9001vdp", ENDIANNESS_BIG, 16,14, 0, NULL, *ADDRESS_MAP_NAME(gp9001vdp_map)),
	  m_gfxregion(0)
{
}

void gp9001vdp_device::static_set_gfx_region(device_t &device, int gfxregion)
{
	gp9001vdp_device &vdp = downcast<gp9001vdp_device &>(device);
	vdp.m_gfxregion = gfxregion;
}

void gp9001vdp_device::device_validity_check(validity_checker &valid) const
{
}

const address_space_config *gp9001vdp_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

TILE_GET_INFO_MEMBER(gp9001vdp_device::get_top0_tile_info)
{
	int color, tile_number, attrib;

	attrib = top.vram16[2*tile_index];

	tile_number = top.vram16[2*tile_index+1];

	if (gp9001_gfxrom_is_banked)
	{
		tile_number = ( gp9001_gfxrom_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO_MEMBER(
			tile_region,
			tile_number,
			color,
			0);
	//tileinfo.category = (attrib & 0x0f00) >> 8;
}

TILE_GET_INFO_MEMBER(gp9001vdp_device::get_fg0_tile_info)
{
	int color, tile_number, attrib;

	attrib = fg.vram16[2*tile_index];

	tile_number = fg.vram16[2*tile_index+1];


	if (gp9001_gfxrom_is_banked)
	{
		tile_number = ( gp9001_gfxrom_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO_MEMBER(
			tile_region,
			tile_number,
			color,
			0);
	//tileinfo.category = (attrib & 0x0f00) >> 8;
}

TILE_GET_INFO_MEMBER(gp9001vdp_device::get_bg0_tile_info)
{
	int color, tile_number, attrib;
	attrib = bg.vram16[2*tile_index];

	tile_number = bg.vram16[2*tile_index+1];

	if (gp9001_gfxrom_is_banked)
	{
		tile_number = ( gp9001_gfxrom_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO_MEMBER(
			tile_region,
			tile_number,
			color,
			0);
	//tileinfo.category = (attrib & 0x0f00) >> 8;
}

void gp9001vdp_device::create_tilemaps(int region)
{
	tile_region = region;

	top.tmap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(gp9001vdp_device::get_top0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	fg.tmap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(gp9001vdp_device::get_fg0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	bg.tmap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(gp9001vdp_device::get_bg0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);

	top.tmap->set_transparent_pen(0);
	fg.tmap->set_transparent_pen(0);
	bg.tmap->set_transparent_pen(0);
}


void gp9001vdp_device::device_start()
{
	top.vram16 = auto_alloc_array_clear(machine(), UINT16, GP9001_TOP_VRAM_SIZE/2);
	fg.vram16 = auto_alloc_array_clear(machine(), UINT16, GP9001_FG_VRAM_SIZE/2);
	bg.vram16 = auto_alloc_array_clear(machine(), UINT16, GP9001_BG_VRAM_SIZE/2);

	sp.vram16 = auto_alloc_array_clear(machine(), UINT16, GP9001_SPRITERAM_SIZE/2);
	sp.vram16_buffer = auto_alloc_array_clear(machine(), UINT16, GP9001_SPRITERAM_SIZE/2);

	create_tilemaps(m_gfxregion);

	save_pointer(NAME(sp.vram16), GP9001_SPRITERAM_SIZE/2);
	save_pointer(NAME(sp.vram16_buffer), GP9001_SPRITERAM_SIZE/2);
	save_pointer(NAME(top.vram16), GP9001_TOP_VRAM_SIZE/2);
	save_pointer(NAME(fg.vram16), GP9001_FG_VRAM_SIZE/2);
	save_pointer(NAME(bg.vram16), GP9001_BG_VRAM_SIZE/2);

	save_item(NAME(gp9001_scroll_reg));
	save_item(NAME(gp9001_voffs));
	save_item(NAME(bg.scrollx));
	save_item(NAME(bg.scrolly));
	save_item(NAME(fg.scrollx));
	save_item(NAME(fg.scrolly));
	save_item(NAME(top.scrollx));
	save_item(NAME(top.scrolly));
	save_item(NAME(sp.scrollx));
	save_item(NAME(sp.scrolly));
	save_item(NAME(bg.flip));
	save_item(NAME(fg.flip));
	save_item(NAME(top.flip));
	save_item(NAME(sp.flip));

	gp9001_gfxrom_is_banked = 0;
	gp9001_gfxrom_bank_dirty = 0;
	save_item(NAME(gp9001_gfxrom_bank));

	// default layer offsets used by all original games
	bg.extra_xoffset.normal  = -0x1d6;
	bg.extra_xoffset.flipped = -0x229;
	bg.extra_yoffset.normal  = -0x1ef;
	bg.extra_yoffset.flipped = -0x210;

	fg.extra_xoffset.normal  = -0x1d8;
	fg.extra_xoffset.flipped = -0x227;
	fg.extra_yoffset.normal  = -0x1ef;
	fg.extra_yoffset.flipped = -0x210;

	top.extra_xoffset.normal = -0x1da;
	top.extra_xoffset.flipped= -0x225;
	top.extra_yoffset.normal = -0x1ef;
	top.extra_yoffset.flipped= -0x210;

	sp.extra_xoffset.normal  = -0x1cc;
	sp.extra_xoffset.flipped = -0x17b;
	sp.extra_yoffset.normal  = -0x1ef;
	sp.extra_yoffset.flipped = -0x108;

	sp.use_sprite_buffer = 1;
}

void gp9001vdp_device::device_reset()
{
	gp9001_voffs = 0;
	gp9001_scroll_reg = 0;
	bg.scrollx = bg.scrolly = 0;
	fg.scrollx = fg.scrolly = 0;
	top.scrollx = top.scrolly = 0;
	sp.scrollx = sp.scrolly = 0;

	bg.flip = 0;
	fg.flip = 0;
	top.flip = 0;
	sp.flip = 0;

	init_scroll_regs();
}


static void gp9001_voffs_w(gp9001vdp_device *vdp, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&vdp->gp9001_voffs);
}

static int gp9001_videoram16_r(gp9001vdp_device *vdp, offs_t offset)
{
	int offs = vdp->gp9001_voffs;
	vdp->gp9001_voffs++;
	return vdp->space().read_word(offs*2);
}


static void gp9001_videoram16_w(gp9001vdp_device *vdp, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	int offs = vdp->gp9001_voffs;
	vdp->gp9001_voffs++;
	vdp->space().write_word(offs*2, data, mem_mask);
}

static WRITE16_DEVICE_HANDLER( gp9001_devvoffs_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device *)device;

	gp9001_voffs_w(vdp, offset, data, mem_mask);
}


static READ16_DEVICE_HANDLER( gp9001_devvideoram16_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device *)device;
	return gp9001_videoram16_r(vdp, offset);
}

static WRITE16_DEVICE_HANDLER( gp9001_devvideoram16_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device *)device;
	gp9001_videoram16_w(vdp, offset, data, mem_mask);
}


static READ16_DEVICE_HANDLER( gp9001_vdpstatus_r )
{
	return ((space.machine().primary_screen->vpos() + 15) % 262) >= 245;
}

static WRITE16_DEVICE_HANDLER( gp9001_scroll_reg_select_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device *)device;

	if (ACCESSING_BITS_0_7)
	{
		vdp->gp9001_scroll_reg = data & 0x8f;
		if (data & 0x70)
			logerror("Hmmm, selecting unknown LSB video control register (%04x)  Video controller %01x  \n",vdp->gp9001_scroll_reg,vdp->tile_region>>1);
	}
	else
	{
		logerror("Hmmm, selecting unknown MSB video control register (%04x)  Video controller %01x  \n",vdp->gp9001_scroll_reg,vdp->tile_region>>1);
	}
}

static void gp9001_set_scrollx_and_flip_reg(gp9001tilemaplayer* layer, UINT16 data, UINT16 mem_mask, int flip)
{
	COMBINE_DATA(&layer->scrollx);

	if (flip)
	{
		layer->flip |= TILEMAP_FLIPX;
		layer->tmap->set_scrollx(0,-(layer->scrollx+layer->extra_xoffset.flipped));
	}
	else
	{
		layer->flip &= (~TILEMAP_FLIPX);
		layer->tmap->set_scrollx(0,layer->scrollx+layer->extra_xoffset.normal);
	}
	layer->tmap->set_flip(layer->flip);
}

static void gp9001_set_scrolly_and_flip_reg(gp9001tilemaplayer* layer, UINT16 data, UINT16 mem_mask, int flip)
{
	COMBINE_DATA(&layer->scrolly);

	if (flip)
	{
		layer->flip |= TILEMAP_FLIPY;
		layer->tmap->set_scrolly(0,-(layer->scrolly+layer->extra_yoffset.flipped));

	}
	else
	{
		layer->flip &= (~TILEMAP_FLIPY);
		layer->tmap->set_scrolly(0,layer->scrolly+layer->extra_yoffset.normal);
	}

	layer->tmap->set_flip(layer->flip);
}

static void gp9001_set_sprite_scrollx_and_flip_reg(gp9001spritelayer* layer, UINT16 data, UINT16 mem_mask, int flip)
{
	if (flip)
	{
		data += layer->extra_xoffset.flipped;
		COMBINE_DATA(&layer->scrollx);
		if (layer->scrollx & 0x8000) layer->scrollx |= 0xfe00;
		else layer->scrollx &= 0x1ff;
		layer->flip |= GP9001_SPRITE_FLIPX;
	}
	else
	{
		data += layer->extra_xoffset.normal;
		COMBINE_DATA(&layer->scrollx);

		if (layer->scrollx & 0x8000) layer->scrollx |= 0xfe00;
		else layer->scrollx &= 0x1ff;
		layer->flip &= (~GP9001_SPRITE_FLIPX);
	}
}

static void gp9001_set_sprite_scrolly_and_flip_reg(gp9001spritelayer* layer, UINT16 data, UINT16 mem_mask, int flip)
{
	if (flip)
	{
		data += layer->extra_yoffset.flipped;
		COMBINE_DATA(&layer->scrolly);
		if (layer->scrolly & 0x8000) layer->scrolly |= 0xfe00;
		else layer->scrolly &= 0x1ff;
		layer->flip |= GP9001_SPRITE_FLIPY;
	}
	else
	{
		data += layer->extra_yoffset.normal;
		COMBINE_DATA(&layer->scrolly);
		if (layer->scrolly & 0x8000) layer->scrolly |= 0xfe00;
		else layer->scrolly &= 0x1ff;
		layer->flip &= (~GP9001_SPRITE_FLIPY);
	}
}

static void gp9001_scroll_reg_data_w(gp9001vdp_device *vdp, offs_t offset, UINT16 data, UINT16 mem_mask)
{

	/************************************************************************/
	/***** layer X and Y flips can be set independently, so emulate it ******/
	/************************************************************************/

	//printf("gp9001_scroll_reg_data_w %04x %04x\n", offset, data);

	// writes with 8x set turn on flip for the specified layer / axis
	int flip = vdp->gp9001_scroll_reg & 0x80;

	switch(vdp->gp9001_scroll_reg&0x7f)
	{
		case 0x00: gp9001_set_scrollx_and_flip_reg(&vdp->bg, data, mem_mask, flip); break;
		case 0x01: gp9001_set_scrolly_and_flip_reg(&vdp->bg, data, mem_mask, flip); break;

		case 0x02: gp9001_set_scrollx_and_flip_reg(&vdp->fg, data, mem_mask, flip); break;
		case 0x03: gp9001_set_scrolly_and_flip_reg(&vdp->fg, data, mem_mask, flip); break;

		case 0x04: gp9001_set_scrollx_and_flip_reg(&vdp->top,data, mem_mask, flip); break;
		case 0x05: gp9001_set_scrolly_and_flip_reg(&vdp->top,data, mem_mask, flip); break;

		case 0x06: gp9001_set_sprite_scrollx_and_flip_reg(&vdp->sp, data,mem_mask,flip); break;
		case 0x07: gp9001_set_sprite_scrolly_and_flip_reg(&vdp->sp, data,mem_mask,flip); break;


		case 0x0e:	/******* Initialise video controller register ? *******/

		case 0x0f:	break;


		default:	logerror("Hmmm, writing %08x to unknown video control register (%08x)  Video controller %01x  !!!\n",data ,vdp->gp9001_scroll_reg,vdp->tile_region>>1);
					break;
	}
}

void gp9001vdp_device::init_scroll_regs()
{
	gp9001_set_scrollx_and_flip_reg(&bg, 0, 0xffff, 0);
	gp9001_set_scrolly_and_flip_reg(&bg, 0, 0xffff, 0);
	gp9001_set_scrollx_and_flip_reg(&fg, 0, 0xffff, 0);
	gp9001_set_scrolly_and_flip_reg(&fg, 0, 0xffff, 0);
	gp9001_set_scrollx_and_flip_reg(&top,0, 0xffff, 0);
	gp9001_set_scrolly_and_flip_reg(&top,0, 0xffff, 0);
	gp9001_set_sprite_scrollx_and_flip_reg(&sp, 0,0xffff,0);
	gp9001_set_sprite_scrolly_and_flip_reg(&sp, 0,0xffff,0);
}


static WRITE16_DEVICE_HANDLER( gp9001_scroll_reg_devvdata_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	gp9001_scroll_reg_data_w(vdp, offset, data, mem_mask);
}



READ16_DEVICE_HANDLER( gp9001_vdp_r )
{
	switch (offset)
	{
		case 0x04/2:
		case 0x06/2:
			return gp9001_devvideoram16_r(device, space, offset-0x04/2, mem_mask);

		case 0x0c/2:
			return gp9001_vdpstatus_r(device, space, offset-0x0c/2, mem_mask);

		default:
			logerror("gp9001_vdp_r: read from unhandled offset %04x\n",offset*2);
	}

	return 0xffff;
}

WRITE16_DEVICE_HANDLER( gp9001_vdp_w )
{
	switch (offset)
	{
		case 0x00/2:
			gp9001_devvoffs_w(device, space, offset-0x00/2, data, mem_mask);
			break;

		case 0x04/2:
		case 0x06/2:
			gp9001_devvideoram16_w(device, space, offset-0x04/2, data, mem_mask);
			break;

		case 0x08/2:
			gp9001_scroll_reg_select_w(device, space, offset-0x08/2, data, mem_mask);
			break;

		case 0x0c/2:
			gp9001_scroll_reg_devvdata_w(device, space, offset-0x0c/2, data, mem_mask);
			break;

		default:
			logerror("gp9001_vdp_w: write to unhandled offset %04x %04x\n",offset, data);
			break;
	}
}

/* some raizing games have a different layout */
READ16_DEVICE_HANDLER( gp9001_vdp_alt_r )
{
	switch (offset)
	{
		case 0x00/2:
			return gp9001_vdpstatus_r(device, space, offset-0x0c/2, mem_mask);

		case 0x08/2:
		case 0x0a/2:
			return gp9001_devvideoram16_r(device, space, offset-0x04/2, mem_mask);


		default:
			logerror("gp9001_vdp_alt_r: read from unhandled offset %04x\n",offset*2);
	}

	return 0xffff;
}

WRITE16_DEVICE_HANDLER( gp9001_vdp_alt_w )
{
	switch (offset)
	{
		case 0x00/2:
			gp9001_scroll_reg_devvdata_w(device, space, offset-0x0c/2, data, mem_mask);
			break;

		case 0x04/2:
			gp9001_scroll_reg_select_w(device, space, offset-0x08/2, data, mem_mask);
			break;

		case 0x08/2:
		case 0x0a/2:
			gp9001_devvideoram16_w(device, space, offset-0x04/2, data, mem_mask);
			break;

		case 0x0c/2:
			gp9001_devvoffs_w(device, space, offset-0x00/2, data, mem_mask);
			break;

		default:
			logerror("gp9001_vdp_alt_w: write to unhandled offset %04x %04x\n",offset, data);
			break;
	}
}



/***************************************************************************/
/**************** PIPIBIBI bootleg interface into this video driver ********/

WRITE16_DEVICE_HANDLER( pipibibi_bootleg_scroll_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device *)device;

	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
	{
		switch(offset)
		{
			case 0x00:	data -= 0x01f; break;
			case 0x01:	data += 0x1ef; break;
			case 0x02:	data -= 0x01d; break;
			case 0x03:	data += 0x1ef; break;
			case 0x04:	data -= 0x01b; break;
			case 0x05:	data += 0x1ef; break;
			case 0x06:	data += 0x1d4; break;
			case 0x07:	data += 0x1f7; break;
			default:	logerror("PIPIBIBI writing %04x to unknown scroll register %04x",data, offset);
		}

		vdp->gp9001_scroll_reg = offset;
		gp9001_scroll_reg_data_w(vdp, offset, data, mem_mask);
	}
}

READ16_DEVICE_HANDLER( pipibibi_bootleg_videoram16_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	gp9001_voffs_w(vdp, 0, offset, 0xffff);
	return gp9001_videoram16_r(vdp, 0);
}

WRITE16_DEVICE_HANDLER( pipibibi_bootleg_videoram16_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	gp9001_voffs_w(vdp, 0, offset, 0xffff);
	gp9001_videoram16_w(vdp, 0, data, mem_mask);
}

READ16_DEVICE_HANDLER( pipibibi_bootleg_spriteram16_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	gp9001_voffs_w(vdp, 0, (0x1800 + offset), 0);
	return gp9001_videoram16_r(vdp, 0);
}

WRITE16_DEVICE_HANDLER( pipibibi_bootleg_spriteram16_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	gp9001_voffs_w(vdp, 0, (0x1800 + offset), mem_mask);
	gp9001_videoram16_w(vdp, 0, data, mem_mask);
}

/***************************************************************************
    Sprite Handlers
***************************************************************************/

void gp9001vdp_device::draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, const UINT8* primap )
{
	gfx_element *gfx = machine.gfx[tile_region+1];

	int offs, old_x, old_y;

	UINT16 *source;

	if (sp.use_sprite_buffer) source=(UINT16 *)(sp.vram16_buffer);
	else source=(UINT16 *)(sp.vram16);

	old_x = (-(sp.scrollx)) & 0x1ff;
	old_y = (-(sp.scrolly)) & 0x1ff;


	for (offs = 0; offs < (GP9001_SPRITERAM_SIZE/2); offs += 4)
	{
		int attrib, sprite, color, priority, flipx, flipy, sx, sy;
		int sprite_sizex, sprite_sizey, dim_x, dim_y, sx_base, sy_base;
		int bank, sprite_num;
		UINT16 primask = (GP9001_PRIMASK << 8);

		attrib = source[offs];
		priority = primap[((attrib & primask)>>8)]+1;

		if ((attrib & 0x8000))
		{
			if (!gp9001_gfxrom_is_banked)	/* No Sprite select bank switching needed */
			{
				sprite = ((attrib & 3) << 16) | source[offs + 1];	/* 18 bit */
			}
			else		/* Batrider Sprite select bank switching required */
			{
				sprite_num = source[offs + 1] & 0x7fff;
				bank = ((attrib & 3) << 1) | (source[offs + 1] >> 15);
				sprite = (gp9001_gfxrom_bank[bank] << 15 ) | sprite_num;
			}
			color = (attrib >> 2) & 0x3f;

			/***** find out sprite size *****/
			sprite_sizex = ((source[offs + 2] & 0x0f) + 1) * 8;
			sprite_sizey = ((source[offs + 3] & 0x0f) + 1) * 8;

			/***** find position to display sprite *****/
			if (!(attrib & 0x4000))
			{
				sx_base = ((source[offs + 2] >> 7) - (sp.scrollx)) & 0x1ff;
				sy_base = ((source[offs + 3] >> 7) - (sp.scrolly)) & 0x1ff;

			} else {
				sx_base = (old_x + (source[offs + 2] >> 7)) & 0x1ff;
				sy_base = (old_y + (source[offs + 3] >> 7)) & 0x1ff;
			}

			old_x = sx_base;
			old_y = sy_base;

			flipx = attrib & GP9001_SPRITE_FLIPX;
			flipy = attrib & GP9001_SPRITE_FLIPY;

			if (flipx)
			{
				/***** Wrap sprite position around *****/
				sx_base -= 7;
				if (sx_base >= 0x1c0) sx_base -= 0x200;
			}
			else
			{
				if (sx_base >= 0x180) sx_base -= 0x200;
			}

			if (flipy)
			{
				sy_base -= 7;
				if (sy_base >= 0x1c0) sy_base -= 0x200;
			}
			else
			{
				if (sy_base >= 0x180) sy_base -= 0x200;
			}

			/***** Flip the sprite layer in any active X or Y flip *****/
			if (sp.flip)
			{
				if (sp.flip & GP9001_SPRITE_FLIPX)
					sx_base = 320 - sx_base;
				if (sp.flip & GP9001_SPRITE_FLIPY)
					sy_base = 240 - sy_base;
			}

			/***** Cancel flip, if it, and sprite layer flip are active *****/
			flipx = (flipx ^ (sp.flip & GP9001_SPRITE_FLIPX));
			flipy = (flipy ^ (sp.flip & GP9001_SPRITE_FLIPY));

			/***** Draw the complete sprites using the dimension info *****/
			for (dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				if (flipy) sy = sy_base - dim_y;
				else       sy = sy_base + dim_y;
				for (dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					if (flipx) sx = sx_base - dim_x;
					else       sx = sx_base + dim_x;

					/*
                    drawgfx_transpen(bitmap,cliprect,gfx,sprite,
                        color,
                        flipx,flipy,
                        sx,sy,0);
                    */
					sprite %= gfx->elements();
					color %= gfx->colors();

					{
						int yy, xx;
						const pen_t *paldata = &gfx->machine().pens[gfx->colorbase() + gfx->granularity() * color];
						const UINT8* srcdata = gfx->get_data(sprite);
						int count = 0;
						int ystart, yend, yinc;
						int xstart, xend, xinc;

						if (flipy)
						{
							ystart = 7;
							yend = -1;
							yinc = -1;
						}
						else
						{
							ystart = 0;
							yend = 8;
							yinc = 1;
						}

						if (flipx)
						{
							xstart = 7;
							xend = -1;
							xinc = -1;
						}
						else
						{
							xstart = 0;
							xend = 8;
							xinc = 1;
						}

						for (yy=ystart;yy!=yend;yy+=yinc)
						{
							int drawyy = yy+sy;


							for (xx=xstart;xx!=xend;xx+=xinc)
							{
								int drawxx = xx+sx;

								if (cliprect.contains(drawxx, drawyy))
								{
									UINT8 pix = srcdata[count];
									UINT16* dstptr = &bitmap.pix16(drawyy, drawxx);
									UINT8* dstpri = &this->custom_priority_bitmap->pix8(drawyy, drawxx);

									if (priority >= dstpri[0])
									{
										if (pix&0xf)
										{
											dstptr[0] = paldata[pix];
											dstpri[0] = priority;

										}
									}
								}


								count++;
							}
						}


					}

					sprite++ ;
				}
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_ind16.
***************************************************************************/

void gp9001vdp_device::gp9001_draw_custom_tilemap(running_machine& machine, bitmap_ind16 &bitmap, tilemap_t* tilemap, const UINT8* priremap, const UINT8* pri_enable )
{
	int width = machine.primary_screen->width();
	int height = machine.primary_screen->height();
	int y,x;
	bitmap_ind16 &tmb = tilemap->pixmap();
	UINT16* srcptr;
	UINT16* dstptr;
	UINT8* dstpriptr;

	int scrollx = tilemap->scrollx(0);
	int scrolly = tilemap->scrolly(0);

	for (y=0;y<height;y++)
	{
		int realy = (y+scrolly)&0x1ff;

		srcptr = &tmb.pix16(realy);
		dstptr = &bitmap.pix16(y);
		dstpriptr = &this->custom_priority_bitmap->pix8(y);

		for (x=0;x<width;x++)
		{
			int realx = (x+scrollx)&0x1ff;

			UINT16 pixdat = srcptr[realx];
			UINT8 pixpri = ((pixdat & (GP9001_PRIMASK<<12))>>12);

			if (pri_enable[pixpri])
			{
				pixpri = priremap[pixpri]+1; // priority of 0 isn't desireable
				pixdat &=0x07ff;

				if (pixdat&0xf)
				{
					if (pixpri >= dstpriptr[x])
					{
						dstptr[x] = pixdat;
						dstpriptr[x] = pixpri;
					}
				}
			}
		}
	}
}


static const UINT8 gp9001_primap1[16] =  { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c };
//static const UINT8 gp9001_sprprimap1[16] =  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static const UINT8 gp9001_sprprimap1[16] =  { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c };

static const UINT8 batsugun_prienable0[16]={ 1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1 };

void gp9001vdp_device::gp9001_render_vdp(running_machine& machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (gp9001_gfxrom_is_banked && gp9001_gfxrom_bank_dirty)
	{
		bg.tmap->mark_all_dirty();
		fg.tmap->mark_all_dirty();
		gp9001_gfxrom_bank_dirty = 0;
	}

	gp9001_draw_custom_tilemap( machine, bitmap, bg.tmap, gp9001_primap1, batsugun_prienable0);
	gp9001_draw_custom_tilemap( machine, bitmap, fg.tmap, gp9001_primap1, batsugun_prienable0);
	gp9001_draw_custom_tilemap( machine, bitmap, top.tmap, gp9001_primap1, batsugun_prienable0);
	draw_sprites( machine,bitmap,cliprect, gp9001_sprprimap1);
}


void gp9001vdp_device::gp9001_screen_eof(void)
{
	/** Shift sprite RAM buffers  ***  Used to fix sprite lag **/
	if (sp.use_sprite_buffer) memcpy(sp.vram16_buffer,sp.vram16,GP9001_SPRITERAM_SIZE);
}



