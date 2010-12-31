/* GP9001 Video Controller */

/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use one or more Toaplan L7A0498 GP9001 graphic controllers.

  The simpler hardware of these games use one GP9001 controller.
  Next we have games that use two GP9001 controllers, whose priority
  schemes between the two controllers is unknown at this time, and
  may be game dependant.
  Finally we have games using one GP9001 controller and an additional
  text tile layer, which has highest priority. This text tile layer
  appears to have line-scroll support. Some of these games copy the
  text tile gfx data to RAM from the main CPU ROM, which easily allows
  for effects to be added to the tiles, by manipulating the text tile
  gfx data. The tiles are then dynamically decoded from RAM before
  displaying them.


 To Do / Unknowns
    -  Hack is needed to reset sound CPU and sound chip when machine
        is 'tilted' in Pipi & Bibis. Otherwise sound CPU interferes
        with the main CPU test of shared RAM. You get a 'Sub CPU RAM Error'
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
    -  Batsugun, relationship between the two video controllers (priority
        wise) is wrong and unknown.





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


static WRITE16_DEVICE_HANDLER( gp9001_bg_tilemap_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->bgvideoram16[offset]);
	tilemap_mark_tile_dirty(vdp->bg_tilemap,offset/2);
}

static WRITE16_DEVICE_HANDLER( gp9001_fg_tilemap_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->fgvideoram16[offset]);
	tilemap_mark_tile_dirty(vdp->fg_tilemap,offset/2);
}

static WRITE16_DEVICE_HANDLER( gp9001_top_tilemap_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->topvideoram16[offset]);
	tilemap_mark_tile_dirty(vdp->top_tilemap,offset/2);
}

static READ16_DEVICE_HANDLER( gp9001_bg_tilemap_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->bgvideoram16[offset];
}

static READ16_DEVICE_HANDLER( gp9001_fg_tilemap_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->fgvideoram16[offset];
}

static READ16_DEVICE_HANDLER( gp9001_top_tilemap_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->topvideoram16[offset];
}

static READ16_DEVICE_HANDLER( gp9001_spram_r )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	return vdp->spriteram16_new[offset];
}

static WRITE16_DEVICE_HANDLER( gp9001_spram_w )
{
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;
	COMBINE_DATA(&vdp->spriteram16_new[offset]);
}

static ADDRESS_MAP_START( gp9001vdp_map, 0, 16 )
	AM_RANGE(0x0000, 0x0fff) AM_DEVREADWRITE(DEVICE_SELF, gp9001_bg_tilemap_r, gp9001_bg_tilemap_w)
	AM_RANGE(0x1000, 0x1fff) AM_DEVREADWRITE(DEVICE_SELF, gp9001_fg_tilemap_r, gp9001_fg_tilemap_w)
	AM_RANGE(0x2000, 0x2fff) AM_DEVREADWRITE(DEVICE_SELF, gp9001_top_tilemap_r, gp9001_top_tilemap_w)
	AM_RANGE(0x3000, 0x37ff) AM_DEVREADWRITE(DEVICE_SELF, gp9001_spram_r, gp9001_spram_w)
	AM_RANGE(0x3800, 0x3fff) AM_RAM // sprite mirror?
ADDRESS_MAP_END


gp9001vdp_device_config::gp9001vdp_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "gp9001vdp_", tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("gp9001vdp", ENDIANNESS_BIG, 16,14, 0, NULL, *ADDRESS_MAP_NAME(gp9001vdp_map))
{
}

device_config *gp9001vdp_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(gp9001vdp_device_config(mconfig, tag, owner, clock));
}

device_t *gp9001vdp_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, gp9001vdp_device(machine, *this));
}

void gp9001vdp_device_config::static_set_gfx_region(device_config *device, int gfxregion)
{
	gp9001vdp_device_config *vdp = downcast<gp9001vdp_device_config *>(device);
	vdp->m_gfxregion = gfxregion;
}

bool gp9001vdp_device_config::device_validity_check(const game_driver &driver) const
{
	bool error = false;
	return error;
}

const address_space_config *gp9001vdp_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


gp9001vdp_device::gp9001vdp_device(running_machine &_machine, const gp9001vdp_device_config &config)
	: device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
	  m_config(config),
	  m_gfxregion(m_config.m_gfxregion)
{
}



static TILE_GET_INFO_DEVICE( get_top0_tile_info )
{
	int color, tile_number, attrib;

	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	attrib = vdp->topvideoram16[2*tile_index];

	tile_number = vdp->topvideoram16[2*tile_index+1];

	if (vdp->gp9001_gfxrom_is_banked)
	{
		tile_number = ( vdp->gp9001_gfxrom_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO_DEVICE(
			vdp->tile_region,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}



static TILE_GET_INFO_DEVICE( get_fg0_tile_info )
{
	int color, tile_number, attrib;

	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	attrib = vdp->fgvideoram16[2*tile_index];

	tile_number = vdp->fgvideoram16[2*tile_index+1];


	if (vdp->gp9001_gfxrom_is_banked)
	{
		tile_number = ( vdp->gp9001_gfxrom_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO_DEVICE(
			vdp->tile_region,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

static TILE_GET_INFO_DEVICE( get_bg0_tile_info )
{
	int color, tile_number, attrib;
	gp9001vdp_device *vdp = (gp9001vdp_device*)device;

	attrib = vdp->bgvideoram16[2*tile_index];

	tile_number = vdp->bgvideoram16[2*tile_index+1];

	if (vdp->gp9001_gfxrom_is_banked)
	{
		tile_number = ( vdp->gp9001_gfxrom_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO_DEVICE(
			vdp->tile_region,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

void gp9001vdp_device::create_tilemaps(int region)
{
	tile_region = region;

	top_tilemap = tilemap_create_device(this, get_top0_tile_info,tilemap_scan_rows,16,16,32,32);
	fg_tilemap = tilemap_create_device(this, get_fg0_tile_info,tilemap_scan_rows,16,16,32,32);
	bg_tilemap = tilemap_create_device(this, get_bg0_tile_info,tilemap_scan_rows,16,16,32,32);

	tilemap_set_transparent_pen(top_tilemap,0);
	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(bg_tilemap,0);
}


void gp9001vdp_device::device_start()
{
	topvideoram16 = auto_alloc_array_clear(machine, UINT16, GP9001_TOP_VRAM_SIZE/2);
	fgvideoram16 = auto_alloc_array_clear(machine, UINT16, GP9001_FG_VRAM_SIZE/2);
	bgvideoram16 = auto_alloc_array_clear(machine, UINT16, GP9001_BG_VRAM_SIZE/2);

	spriteram16_new = auto_alloc_array_clear(machine, UINT16, GP9001_SPRITERAM_SIZE/2);
	spriteram16_now = auto_alloc_array_clear(machine, UINT16, GP9001_SPRITERAM_SIZE/2);

	spriteram16_n = spriteram16_now;

	create_tilemaps(m_gfxregion);

	state_save_register_device_item_pointer(this, 0, spriteram16_new, GP9001_SPRITERAM_SIZE/2);
	state_save_register_device_item_pointer(this, 0, spriteram16_now, GP9001_SPRITERAM_SIZE/2);
	state_save_register_device_item_pointer(this, 0, topvideoram16, GP9001_TOP_VRAM_SIZE/2);
	state_save_register_device_item_pointer(this, 0, fgvideoram16, GP9001_FG_VRAM_SIZE/2);
	state_save_register_device_item_pointer(this, 0, bgvideoram16, GP9001_BG_VRAM_SIZE/2);

	state_save_register_device_item(this,0, gp9001_scroll_reg);
	state_save_register_device_item(this,0, gp9001_voffs);
	state_save_register_device_item(this,0, bg_scrollx);
	state_save_register_device_item(this,0, bg_scrolly);
	state_save_register_device_item(this,0, fg_scrollx);
	state_save_register_device_item(this,0, fg_scrolly);
	state_save_register_device_item(this,0, top_scrollx);
	state_save_register_device_item(this,0, top_scrolly);
	state_save_register_device_item(this,0, sprite_scrollx);
	state_save_register_device_item(this,0, sprite_scrolly);
	state_save_register_device_item(this,0, bg_flip);
	state_save_register_device_item(this,0, fg_flip);
	state_save_register_device_item(this,0, top_flip);
	state_save_register_device_item(this,0, sprite_flip);

	gp9001_gfxrom_is_banked = 0;
	gp9001_gfxrom_bank_dirty = 0;
	state_save_register_device_item_array(this,0,gp9001_gfxrom_bank);

	extra_xoffset[0]=0;
	extra_xoffset[1]=0;
	extra_xoffset[2]=0;
	extra_xoffset[3]=0;

	extra_yoffset[0]=0;
	extra_yoffset[1]=0;
	extra_yoffset[2]=0;
	extra_yoffset[3]=0;
}

void gp9001vdp_device::device_reset()
{
	gp9001_voffs = 0;
	gp9001_scroll_reg = 0;
	bg_scrollx = bg_scrolly = 0;
	fg_scrollx = fg_scrolly = 0;
	top_scrollx = top_scrolly = 0;
	sprite_scrollx = sprite_scrolly = 0;

	bg_flip = 0;
	fg_flip = 0;
	top_flip = 0;
	sprite_flip = 0;

	/* debug */
	display_bg = 1;
	display_fg = 1;
	display_top = 1;
	display_sp = 1;
}


static void gp9001_voffs_w(gp9001vdp_device *vdp, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&vdp->gp9001_voffs);
}

static int gp9001_videoram16_r(gp9001vdp_device *vdp, offs_t offset)
{
	int offs = vdp->gp9001_voffs;
	vdp->gp9001_voffs++;
	return vdp->space()->read_word(offs*2);
}


static void gp9001_videoram16_w(gp9001vdp_device *vdp, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	int offs = vdp->gp9001_voffs;
	vdp->gp9001_voffs++;
	vdp->space()->write_word(offs*2, data, mem_mask);
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
	return ((device->machine->primary_screen->vpos() + 15) % 262) >= 245;
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

static void gp9001_scroll_reg_data_w(gp9001vdp_device *vdp, offs_t offset, UINT16 data, UINT16 mem_mask)
{

	/************************************************************************/
	/***** layer X and Y flips can be set independently, so emulate it ******/
	/************************************************************************/

	//printf("gp9001_scroll_reg_data_w %04x %04x\n", offset, data);


	switch(vdp->gp9001_scroll_reg)
	{
		case 0x00:	data -= 0x1d6;			/* 1D6h */
					COMBINE_DATA(&vdp->bg_scrollx);
					vdp->bg_flip &= (~TILEMAP_FLIPX);
					tilemap_set_flip(vdp->bg_tilemap,vdp->bg_flip);
					tilemap_set_scrollx(vdp->bg_tilemap,0,vdp->bg_scrollx+vdp->extra_xoffset[0]);
					break;
		case 0x01:	data -= 0x1ef;			/* 1EFh */
					COMBINE_DATA(&vdp->bg_scrolly);
					vdp->bg_flip &= (~TILEMAP_FLIPY);
					tilemap_set_flip(vdp->bg_tilemap,vdp->bg_flip);
					tilemap_set_scrolly(vdp->bg_tilemap,0,vdp->bg_scrolly+vdp->extra_yoffset[0]);
					break;
		case 0x02:	data -= 0x1d8;			/* 1D0h */
					COMBINE_DATA(&vdp->fg_scrollx);
					vdp->fg_flip &= (~TILEMAP_FLIPX);
					tilemap_set_flip(vdp->fg_tilemap,vdp->fg_flip);
					tilemap_set_scrollx(vdp->fg_tilemap,0,vdp->fg_scrollx+vdp->extra_xoffset[1]);
					break;
		case 0x03:  data -= 0x1ef;			/* 1EFh */
					COMBINE_DATA(&vdp->fg_scrolly);
					vdp->fg_flip &= (~TILEMAP_FLIPY);
					tilemap_set_flip(vdp->fg_tilemap,vdp->fg_flip);
					tilemap_set_scrolly(vdp->fg_tilemap,0,vdp->fg_scrolly+vdp->extra_yoffset[1]);
					break;
		case 0x04:	data -= 0x1da;			/* 1DAh */
					COMBINE_DATA(&vdp->top_scrollx);
					vdp->top_flip &= (~TILEMAP_FLIPX);
					tilemap_set_flip(vdp->top_tilemap,vdp->top_flip);
					tilemap_set_scrollx(vdp->top_tilemap,0,vdp->top_scrollx+vdp->extra_xoffset[2]);
					break;
		case 0x05:	data -= 0x1ef;			/* 1EFh */
					COMBINE_DATA(&vdp->top_scrolly);
					vdp->top_flip &= (~TILEMAP_FLIPY);
					tilemap_set_flip(vdp->top_tilemap,vdp->top_flip);
					tilemap_set_scrolly(vdp->top_tilemap,0,vdp->top_scrolly+vdp->extra_yoffset[2]);
					break;
		case 0x06:  data -= 0x1cc;			/* 1D4h */
					COMBINE_DATA(&vdp->sprite_scrollx);
					if (vdp->sprite_scrollx & 0x8000) vdp->sprite_scrollx |= 0xfffffe00;
					else vdp->sprite_scrollx &= 0x1ff;
					vdp->sprite_flip &= (~GP9001_SPRITE_FLIPX);
					break;
		case 0x07:	data -= 0x1ef;      /* 1F7h */
					COMBINE_DATA(&vdp->sprite_scrolly);
					if (vdp->sprite_scrolly & 0x8000) vdp->sprite_scrolly |= 0xfffffe00;
					else vdp->sprite_scrolly &= 0x1ff;
					vdp->sprite_flip &= (~GP9001_SPRITE_FLIPY);
					break;
		case 0x0f:	break;
		case 0x80:  data -= 0x229;			/* 169h */
					COMBINE_DATA(&vdp->bg_scrollx);
					vdp->bg_flip |= TILEMAP_FLIPX;
					tilemap_set_flip(vdp->bg_tilemap,vdp->bg_flip);
					tilemap_set_scrollx(vdp->bg_tilemap,0,vdp->bg_scrollx+vdp->extra_xoffset[0]);
					break;
		case 0x81:	data -= 0x210;			/* 100h */
					COMBINE_DATA(&vdp->bg_scrolly);
					vdp->bg_flip |= TILEMAP_FLIPY;
					tilemap_set_flip(vdp->bg_tilemap,vdp->bg_flip);
					tilemap_set_scrolly(vdp->bg_tilemap,0,vdp->bg_scrolly+vdp->extra_yoffset[0]);
					break;
		case 0x82:	data -= 0x227;			/* 15Fh */
					COMBINE_DATA(&vdp->fg_scrollx);
					vdp->fg_flip |= TILEMAP_FLIPX;
					tilemap_set_flip(vdp->fg_tilemap,vdp->fg_flip);
					tilemap_set_scrollx(vdp->fg_tilemap,0,vdp->fg_scrollx+vdp->extra_xoffset[1]);
					break;
		case 0x83:	data -= 0x210;			/* 100h */
					COMBINE_DATA(&vdp->fg_scrolly);
					vdp->fg_flip |= TILEMAP_FLIPY;
					tilemap_set_flip(vdp->fg_tilemap,vdp->fg_flip);
					tilemap_set_scrolly(vdp->fg_tilemap,0,vdp->fg_scrolly+vdp->extra_yoffset[1]);
					break;
		case 0x84:	data -= 0x225;			/* 165h */
					COMBINE_DATA(&vdp->top_scrollx);
					vdp->top_flip |= TILEMAP_FLIPX;
					tilemap_set_flip(vdp->top_tilemap,vdp->top_flip);
					tilemap_set_scrollx(vdp->top_tilemap,0,vdp->top_scrollx+vdp->extra_xoffset[2]);
					break;
		case 0x85:	data -= 0x210;			/* 100h */
					COMBINE_DATA(&vdp->top_scrolly);
					vdp->top_flip |= TILEMAP_FLIPY;
					tilemap_set_flip(vdp->top_tilemap,vdp->top_flip);
					tilemap_set_scrolly(vdp->top_tilemap,0,vdp->top_scrolly+vdp->extra_yoffset[2]);
					break;
		case 0x86:	data -= 0x17b;			/* 17Bh */
					COMBINE_DATA(&vdp->sprite_scrollx);
					if (vdp->sprite_scrollx & 0x8000) vdp->sprite_scrollx |= 0xfffffe00;
					else vdp->sprite_scrollx &= 0x1ff;
					vdp->sprite_flip |= GP9001_SPRITE_FLIPX;
					break;
		case 0x87:	data -= 0x108;			/* 108h */
					COMBINE_DATA(&vdp->sprite_scrolly);
					if (vdp->sprite_scrolly & 0x8000) vdp->sprite_scrolly |= 0xfffffe00;
					else vdp->sprite_scrolly &= 0x1ff;
					vdp->sprite_flip |= GP9001_SPRITE_FLIPY;
					break;
		case 0x8f:	break;

		case 0x0e:	/******* Initialise video controller register ? *******/
					#if 0 // do we know this works on real hw?
					if ((gp9001_sub_cpu == CPU_2_Z80) && (data == 3))
					{
						/* HACK! When tilted, sound CPU needs to be reset. */
						device_t *ym = vdp->machine->device("ymsnd");

						if (ym && ym->type() == YM3812)
						{
							cputag_set_input_line(vdp->machine, "audiocpu", INPUT_LINE_RESET, PULSE_LINE);
							devtag_reset(vdp->machine, "ymsnd");
						}
					}
					#endif

		default:	logerror("Hmmm, writing %08x to unknown video control register (%08x)  Video controller %01x  !!!\n",data ,vdp->gp9001_scroll_reg,vdp->tile_region>>1);
					break;
	}

// enable / disable layer debug keys (broken at the moment)
#ifdef MAME_DEBUG
	// 1st vdp
	if (vdp->tile_region == 0)
	{
		/* this is non-vdp
        if ( input_code_pressed_once(vdp->machine, KEYCODE_W) )
        {
            display_tx += 1;
            display_tx &= 1;
            if (gp9001_txvideoram16 != 0)
                tilemap_set_enable(tx_tilemap, display_tx);
        }
        */

		if ( input_code_pressed_once(vdp->machine, KEYCODE_L) )
		{
			vdp->display_sp += 1;
			vdp->display_sp &= 1;
		}
		if ( input_code_pressed_once(vdp->machine, KEYCODE_K) )
		{
			vdp->display_top += 1;
			vdp->display_top &= 1;
			tilemap_set_enable(vdp->top_tilemap, vdp->display_top);
		}
		if ( input_code_pressed_once(vdp->machine, KEYCODE_J) )
		{
			vdp->display_fg += 1;
			vdp->display_fg &= 1;
			tilemap_set_enable(vdp->fg_tilemap, vdp->display_fg);
		}
		if ( input_code_pressed_once(vdp->machine, KEYCODE_H) )
		{
			vdp->display_bg += 1;
			vdp->display_bg &= 1;
			tilemap_set_enable(vdp->bg_tilemap, vdp->display_bg);
		}
	}

	// 2nd vdp
	if (vdp->tile_region == 2)
	{
		if ( input_code_pressed_once(vdp->machine, KEYCODE_O) )
		{
			vdp->display_sp += 1;
			vdp->display_sp &= 1;
		}
		if ( input_code_pressed_once(vdp->machine, KEYCODE_I) )
		{
			vdp->display_top += 1;
			vdp->display_top &= 1;
			tilemap_set_enable(vdp->top_tilemap, vdp->display_top);
		}
		if ( input_code_pressed_once(vdp->machine, KEYCODE_U) )
		{
			vdp->display_fg += 1;
			vdp->display_fg &= 1;
			tilemap_set_enable(vdp->fg_tilemap, vdp->display_fg);
		}
		if ( input_code_pressed_once(vdp->machine, KEYCODE_Y) )
		{
			vdp->display_bg += 1;
			vdp->display_bg &= 1;
			tilemap_set_enable(vdp->bg_tilemap, vdp->display_bg);
		}
	}
#endif
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
			return gp9001_devvideoram16_r(device, offset-0x04/2, mem_mask);

		case 0x0c/2:
			return gp9001_vdpstatus_r(device, offset-0x0c/2, mem_mask);

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
			gp9001_devvoffs_w(device, offset-0x00/2, data, mem_mask);
			break;

		case 0x04/2:
		case 0x06/2:
			gp9001_devvideoram16_w(device, offset-0x04/2, data, mem_mask);
			break;

		case 0x08/2:
			gp9001_scroll_reg_select_w(device, offset-0x08/2, data, mem_mask);
			break;

		case 0x0c/2:
			gp9001_scroll_reg_devvdata_w(device, offset-0x0c/2, data, mem_mask);
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
			return gp9001_vdpstatus_r(device, offset-0x0c/2, mem_mask);

		case 0x08/2:
		case 0x0a/2:
			return gp9001_devvideoram16_r(device, offset-0x04/2, mem_mask);


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
			gp9001_scroll_reg_devvdata_w(device, offset-0x0c/2, data, mem_mask);
			break;

		case 0x04/2:
			gp9001_scroll_reg_select_w(device, offset-0x08/2, data, mem_mask);
			break;

		case 0x08/2:
		case 0x0a/2:
			gp9001_devvideoram16_w(device, offset-0x04/2, data, mem_mask);
			break;

		case 0x0c/2:
			gp9001_devvoffs_w(device, offset-0x00/2, data, mem_mask);
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



void gp9001_log_vram(gp9001vdp_device* vdp, running_machine *machine)
{

#ifdef MAME_DEBUG
	offs_t sprite_voffs, tile_voffs;

	if ( input_code_pressed(machine, KEYCODE_M) )
	{
		UINT16 *source_now0 = 0;
		UINT16 *source_new0 = 0;
		UINT16 *source_now1 = 0;
		UINT16 *source_new1 = 0;

		int schar[2],sattr[2],sxpos[2],sypos[2];

		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");

		if (vdp->tile_region == 0)
		{
			source_now0  = (UINT16 *)(vdp->spriteram16_now);
			source_new0  = (UINT16 *)(vdp->spriteram16_new);
			logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}

		if (vdp->tile_region == 2)
		{
			source_now1  = (UINT16 *)(vdp->spriteram16_now);
			source_new1  = (UINT16 *)(vdp->spriteram16_new);

			logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}


		for ( sprite_voffs = 0; sprite_voffs < (GP9001_SPRITERAM_SIZE/2); sprite_voffs += 4 )
		{
			if (vdp->tile_region == 0)
			{
				sattr[0] = source_now0[sprite_voffs];
				schar[0] = source_now0[sprite_voffs + 1];
				sxpos[0] = source_now0[sprite_voffs + 2];
				sypos[0] = source_now0[sprite_voffs + 3];
				sattr[1] = source_new0[sprite_voffs];
				schar[1] = source_new0[sprite_voffs + 1];
				sxpos[1] = source_new0[sprite_voffs + 2];
				sypos[1] = source_new0[sprite_voffs + 3];
				logerror("SPoffs    Sprt Attr Xpos Ypos     Sprt Attr Xpos Ypos\n");
				logerror("0:%03x now:%04x %04x %04x %04x new:%04x %04x %04x %04x\n",sprite_voffs,
													schar[0], sattr[0],sxpos[0], sypos[0],
													schar[1], sattr[1],sxpos[1], sypos[1]);
			}

			if (vdp->tile_region == 2)
			{
				sattr[0] = source_now1[sprite_voffs];
				schar[0] = source_now1[sprite_voffs + 1];
				sxpos[0] = source_now1[sprite_voffs + 2];
				sypos[0] = source_now1[sprite_voffs + 3];
				sattr[1] = source_new1[sprite_voffs];
				schar[1] = source_new1[sprite_voffs + 1];
				sxpos[1] = source_new1[sprite_voffs + 2];
				sypos[1] = source_new1[sprite_voffs + 3];
				logerror("1:%03x now:%04x %04x %04x %04x new:%04x %04x %04x %04x\n",sprite_voffs,
												schar[0], sattr[0],sxpos[0], sypos[0],
												schar[1], sattr[1],sxpos[1], sypos[1]);
			}
		}
	}

	if ( input_code_pressed(machine, KEYCODE_N) )
	{
		int tchar[2], tattr[2];
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");

		if (vdp->tile_region == 0)
		{
			logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}
		if (vdp->tile_region == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}


		for ( tile_voffs = 0; tile_voffs < (GP9001_TOP_VRAM_SIZE/2); tile_voffs += 2 )
		{
			if (vdp->tile_region == 0)
			{
				tchar[0] = vdp->topvideoram16[tile_voffs + 1];
				tattr[0] = vdp->topvideoram16[tile_voffs];
				logerror("TOPoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[0], tattr[0]);
			}

			if (vdp->tile_region == 2)
			{
				tchar[1] = vdp->topvideoram16[tile_voffs + 1];
				tattr[1] = vdp->topvideoram16[tile_voffs];
				logerror("TOPoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[1], tattr[1]);
			}

		}
	}
	if ( input_code_pressed(machine, KEYCODE_B) )
	{
		int tchar[2], tattr[2];
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");

		if (vdp->tile_region == 0)
		{
			logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}

		if (vdp->tile_region == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}


		for ( tile_voffs = 0; tile_voffs < (GP9001_FG_VRAM_SIZE/2); tile_voffs += 2 )
		{
			if (vdp->tile_region == 0)
			{
				tchar[0] = vdp->fgvideoram16[tile_voffs + 1];
				tattr[0] = vdp->fgvideoram16[tile_voffs];
				logerror("FGoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[0], tattr[0]);
			}


			if (vdp->tile_region == 2)
			{
				tchar[1] = vdp->fgvideoram16[tile_voffs + 1];
				tattr[1] = vdp->fgvideoram16[tile_voffs];
				logerror("FGoffs:%04x   Tile0:%04x  Attr0:%04x \n", tile_voffs, tchar[1], tattr[1]);
			}
		}
	}
	if ( input_code_pressed(machine, KEYCODE_V) )
	{
		int tchar[2], tattr[2];
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");

		if (vdp->tile_region == 0)
		{
			logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}

		if (vdp->tile_region == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}

		for ( tile_voffs = 0; tile_voffs < (GP9001_BG_VRAM_SIZE/2); tile_voffs += 2 )
		{
			if (vdp->tile_region == 0)
			{
				tchar[0] = vdp->bgvideoram16[tile_voffs + 1];
				tattr[0] = vdp->bgvideoram16[tile_voffs];
				logerror("BGoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[0], tattr[0]);
			}

			if (vdp->tile_region == 2)
			{
				tchar[1] = vdp->bgvideoram16[tile_voffs + 1];
				tattr[1] = vdp->bgvideoram16[tile_voffs];
				logerror("BGoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[1], tattr[1]);
			}
		}
	}

	if ( input_code_pressed_once(machine, KEYCODE_C) )
		logerror("Mark here\n");

	if ( input_code_pressed_once(machine, KEYCODE_E) )
	{
		*vdp->displog += 1;
		*vdp->displog &= 1;
	}
	if (*vdp->displog)
	{
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");

		if (vdp->tile_region == 0)
		{
			logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}

		if (vdp->tile_region == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", vdp->bg_scrollx,vdp->bg_scrolly,vdp->fg_scrollx,vdp->fg_scrolly,vdp->top_scrollx,vdp->top_scrolly,vdp->sprite_scrollx, vdp->sprite_scrolly);
		}
	}
#endif
}



/***************************************************************************
    Sprite Handlers
***************************************************************************/

void gp9001vdp_device::draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8* primap )
{
	const gfx_element *gfx = machine->gfx[tile_region+1];

	int offs, old_x, old_y;


	UINT16 *source = (UINT16 *)(spriteram16_n);

	old_x = (-(sprite_scrollx+extra_xoffset[3])) & 0x1ff;
	old_y = (-(sprite_scrolly+extra_yoffset[3])) & 0x1ff;

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
				sx_base = ((source[offs + 2] >> 7) - (sprite_scrollx+extra_xoffset[3])) & 0x1ff;
				sy_base = ((source[offs + 3] >> 7) - (sprite_scrolly+extra_yoffset[3])) & 0x1ff;
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
			if (sprite_flip)
			{
				if (sprite_flip & GP9001_SPRITE_FLIPX)
					sx_base = 320 - sx_base;
				if (sprite_flip & GP9001_SPRITE_FLIPY)
					sy_base = 240 - sy_base;
			}

			/***** Cancel flip, if it, and sprite layer flip are active *****/
			flipx = (flipx ^ (sprite_flip & GP9001_SPRITE_FLIPX));
			flipy = (flipy ^ (sprite_flip & GP9001_SPRITE_FLIPY));

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
					sprite %= gfx->total_elements;
					color %= gfx->total_colors;

					{
						int yy, xx;
						const pen_t *paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];
						const UINT8* srcdata = gfx_element_get_data(gfx, sprite);
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

								if (drawxx>=cliprect->min_x && drawxx<=cliprect->max_x && drawyy>=cliprect->min_y && drawyy<=cliprect->max_y)
								{
									UINT8 pix = srcdata[count];
									UINT16* dstptr = BITMAP_ADDR16(bitmap,drawyy,drawxx);
									UINT8* dstpri = BITMAP_ADDR8(this->custom_priority_bitmap, drawyy, drawxx);

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
    Draw the game screen in the given bitmap_t.
***************************************************************************/

void gp9001vdp_device::gp9001_draw_custom_tilemap(running_machine* machine, bitmap_t* bitmap, tilemap_t* tilemap, const UINT8* priremap, const UINT8* pri_enable )
{
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();
	int y,x;
	bitmap_t *tmb = tilemap_get_pixmap(tilemap);
	UINT16* srcptr;
	UINT16* dstptr;
	UINT8* dstpriptr;

	int scrollx = tilemap_get_scrollx(tilemap, 0);
	int scrolly = tilemap_get_scrolly(tilemap, 0);

	for (y=0;y<height;y++)
	{
		int realy = (y+scrolly)&0x1ff;

		srcptr = BITMAP_ADDR16(tmb, realy, 0);
		dstptr = BITMAP_ADDR16(bitmap, y, 0);
		dstpriptr = BITMAP_ADDR8(this->custom_priority_bitmap, y, 0);

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

void gp9001vdp_device::gp9001_render_vdp(running_machine* machine, bitmap_t* bitmap, const rectangle* cliprect)
{
	if (gp9001_gfxrom_is_banked && gp9001_gfxrom_bank_dirty)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);
		tilemap_mark_all_tiles_dirty(fg_tilemap);
		gp9001_gfxrom_bank_dirty = 0;
	}

	gp9001_draw_custom_tilemap( machine, bitmap, bg_tilemap, gp9001_primap1, batsugun_prienable0);
	gp9001_draw_custom_tilemap( machine, bitmap, fg_tilemap, gp9001_primap1, batsugun_prienable0);
	gp9001_draw_custom_tilemap( machine, bitmap, top_tilemap, gp9001_primap1, batsugun_prienable0);
	draw_sprites( machine,bitmap,cliprect, gp9001_sprprimap1);
}


void gp9001vdp_device::gp9001_video_eof(void)
{
	/** Shift sprite RAM buffers  ***  Used to fix sprite lag **/
	memcpy(spriteram16_now,spriteram16_new,GP9001_SPRITERAM_SIZE);
}



