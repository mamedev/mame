
#include "emu.h"
#include "k001604.h"


/***************************************************************************/
/*                                                                         */
/*                                  001604                                 */
/*                                                                         */
/***************************************************************************/


struct k001604_state
{
	screen_device *screen;
	tilemap_t        *layer_8x8[2];
	tilemap_t        *layer_roz;
	int            gfx_index[2];

	UINT32 *       tile_ram;
	UINT32 *       char_ram;
	UINT32 *       reg;

	int            layer_size;
	int            roz_size;
	int            txt_mem_offset;
	int            roz_mem_offset;
};


#define K001604_NUM_TILES_LAYER0        16384
#define K001604_NUM_TILES_LAYER1        4096

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k001604_state *k001604_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == K001604);

	return (k001604_state *)downcast<k001604_device *>(device)->token();
}

INLINE const k001604_interface *k001604_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == K001604));
	return (const k001604_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static const gfx_layout k001604_char_layout_layer_8x8 =
{
	8, 8,
	K001604_NUM_TILES_LAYER0,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128 },
	8*128
};

static const gfx_layout k001604_char_layout_layer_16x16 =
{
	16, 16,
	K001604_NUM_TILES_LAYER1,
	8,
	{ 8,9,10,11,12,13,14,15 },
	{ 1*16, 0*16, 3*16, 2*16, 5*16, 4*16, 7*16, 6*16, 9*16, 8*16, 11*16, 10*16, 13*16, 12*16, 15*16, 14*16 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256, 8*256, 9*256, 10*256, 11*256, 12*256, 13*256, 14*256, 15*256 },
	16*256
};


/* FIXME: The TILEMAP_MAPPER below depends on parameters passed by the device interface (being game dependent).
we might simplify the code, by passing the whole TILEMAP_MAPPER as a callback in the interface, but is it really worth? */

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_0_size0)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 128) + col + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_0_size1)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 256) + col + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_1_size0)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 128) + col + 64 + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_8x8_1_size1)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 64 + k001604->txt_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_roz_128)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 128) + col + k001604->roz_mem_offset;
}

TILEMAP_MAPPER_MEMBER(k001604_device::k001604_scan_layer_roz_256)
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	/* logical (col,row) -> memory offset */
	return (row * 256) + col + 128 + k001604->roz_mem_offset;
}

TILE_GET_INFO_MEMBER(k001604_device::k001604_tile_info_layer_8x8)
{
	k001604_state *k001604 = k001604_get_safe_token(this);
	UINT32 val = k001604->tile_ram[tile_index];
	int color = (val >> 17) & 0x1f;
	int tile = (val & 0x7fff);
	int flags = 0;

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(k001604->gfx_index[0], tile, color, flags);
}

TILE_GET_INFO_MEMBER(k001604_device::k001604_tile_info_layer_roz)
{
	k001604_state *k001604 = k001604_get_safe_token(this);
	UINT32 val = k001604->tile_ram[tile_index];
	int flags = 0;
	int color = (val >> 17) & 0x1f;
	int tile = k001604->roz_size ? (val & 0x7ff) : (val & 0x1fff);

	if (val & 0x400000)
		flags |= TILE_FLIPX;
	if (val & 0x800000)
		flags |= TILE_FLIPY;

	tile += k001604->roz_size ? 0x800 : 0x2000;

	SET_TILE_INFO_MEMBER(k001604->gfx_index[k001604->roz_size], tile, color, flags);
}


void k001604_draw_back_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	k001604_state *k001604 = k001604_get_safe_token(device);
	bitmap.fill(0, cliprect);

	if ((k001604->reg[0x60 / 4] & 0x40000000) == 0)
		return;

	int tile_size = k001604->roz_size ? 16 : 8;

	INT32 x  = (INT16)((k001604->reg[0x08] >> 16) & 0xffff);
	INT32 y  = (INT16)((k001604->reg[0x08] >>  0) & 0xffff);
	INT32 xx = (INT16)((k001604->reg[0x09] >>  0) & 0xffff);
	INT32 xy = (INT16)((k001604->reg[0x09] >> 16) & 0xffff);
	INT32 yx = (INT16)((k001604->reg[0x0a] >>  0) & 0xffff);
	INT32 yy = (INT16)((k001604->reg[0x0a] >> 16) & 0xffff);

	int pivotx = (INT16)((k001604->reg[0x00] >> 16) & 0xffff);
	int pivoty = (INT16)((k001604->reg[0x00] >>  0) & 0xffff);

	int startx  = ((x - pivotx) * 256) * 32;
	int starty  = ((y - pivoty) * 256) * 32;
	int incxx = (xx) * 32;
	int incxy = (-xy) * 32;
	int incyx = (-yx) * 32;
	int incyy = (yy) * 32;

	bitmap_ind16& pixmap = k001604->layer_roz->pixmap();

	// extract start/end points
	int sx = cliprect.min_x;
	int sy = cliprect.min_y;
	int ex = cliprect.max_x;
	int ey = cliprect.max_y;

	const rgb_t *clut = palette_entry_list_raw(bitmap.palette());

	int window_x, window_y, window_xmask, window_ymask;

	int layer_size = (k001604->reg[0x1b] >> 9) & 3;

	if (k001604->roz_size)
		window_x = ((k001604->reg[0x1b] >> 1) & 3) * 512;
	else
		window_x = ((k001604->reg[0x1b] >> 1) & 1) * 512;

	window_y = 0;

	switch (layer_size)
	{
		case 0: window_xmask = (128 * tile_size) - 1; break;
		case 2: window_xmask = (64 * tile_size) - 1; break;
		case 3: window_xmask = (32 * tile_size) - 1; break;
		default: fatalerror("k001604_draw_back_layer(): layer_size %d\n", layer_size); break;
	}

	window_ymask = pixmap.height() - 1;


	// loop over rows
	while (sy <= ey)
	{
		// initialize X counters
		int x = sx;
		UINT32 cx = startx;
		UINT32 cy = starty;

		UINT32 *dest = &bitmap.pix(sy, sx);

		// loop over columns
		while (x <= ex)
		{
			*dest = clut[pixmap.pix16(((cy >> 16) & window_ymask) + window_y, ((cx >> 16) & window_xmask) + window_x)];

			// advance in X
			cx += incxx;
			cy += incxy;
			x++;
			dest++;
		}

		// advance in Y
		startx += incyx;
		starty += incyy;
		sy++;
	}
}

void k001604_draw_front_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	k001604->layer_8x8[0]->set_scrollx(-cliprect.min_x);
	k001604->layer_8x8[0]->set_scrolly(-cliprect.min_y);

	k001604->layer_8x8[1]->set_scrollx(-cliprect.min_x);
	k001604->layer_8x8[1]->set_scrolly(-cliprect.min_y);

	//k001604->layer_8x8[1]->draw(bitmap, cliprect, 0,0);
	k001604->layer_8x8[0]->draw(bitmap, cliprect, 0,0);
}

READ32_DEVICE_HANDLER( k001604_tile_r )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	return k001604->tile_ram[offset];
}

READ32_DEVICE_HANDLER( k001604_char_r )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	int set, bank;
	UINT32 addr;

	set = (k001604->reg[0x60 / 4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (k001604->reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (k001604->reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	return k001604->char_ram[addr];
}

READ32_DEVICE_HANDLER( k001604_reg_r )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	switch (offset)
	{
		case 0x54/4:    return space.machine().rand() << 16;
		case 0x5c/4:    return space.machine().rand() << 16 | space.machine().rand();
	}

	return k001604->reg[offset];
}

WRITE32_DEVICE_HANDLER( k001604_tile_w )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	int x/*, y*/;
	COMBINE_DATA(k001604->tile_ram + offset);

	if (k001604->layer_size)
	{
		x = offset & 0xff;
		/*y = offset / 256;*/
	}
	else
	{
		x = offset & 0x7f;
		/*y = offset / 128;*/
	}

	if (k001604->layer_size)
	{
		if (x < 64)
		{
			k001604->layer_8x8[0]->mark_tile_dirty(offset);
		}
		else if (x < 128)
		{
			k001604->layer_8x8[1]->mark_tile_dirty(offset);
		}
		else
		{
			k001604->layer_roz->mark_tile_dirty(offset);
		}
	}
	else
	{
		if (x < 64)
		{
			k001604->layer_8x8[0]->mark_tile_dirty(offset);
		}
		else
		{
			k001604->layer_8x8[1]->mark_tile_dirty(offset);
		}

		k001604->layer_roz->mark_tile_dirty(offset);
	}
}

WRITE32_DEVICE_HANDLER( k001604_char_w )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	int set, bank;
	UINT32 addr;

	set = (k001604->reg[0x60/4] & 0x1000000) ? 0x100000 : 0;

	if (set)
		bank = (k001604->reg[0x60 / 4] >> 8) & 0x3;
	else
		bank = (k001604->reg[0x60 / 4] & 0x3);

	addr = offset + ((set + (bank * 0x40000)) / 4);

	COMBINE_DATA(k001604->char_ram + addr);

	space.machine().gfx[k001604->gfx_index[0]]->mark_dirty(addr / 32);
	space.machine().gfx[k001604->gfx_index[1]]->mark_dirty(addr / 128);
}

WRITE32_DEVICE_HANDLER( k001604_reg_w )
{
	k001604_state *k001604 = k001604_get_safe_token(device);

	COMBINE_DATA(k001604->reg + offset);

	switch (offset)
	{
		case 0x8:
		case 0x9:
		case 0xa:
			//printf("K001604_reg_w %02X, %08X, %08X\n", offset, data, mem_mask);
			break;
	}

	if (offset != 0x08 && offset != 0x09 && offset != 0x0a /*&& offset != 0x17 && offset != 0x18*/)
	{
		//printf("K001604_reg_w (%d), %02X, %08X, %08X at %08X\n", chip, offset, data, mem_mask, space.device().safe_pc());
	}
}

const device_type K001604 = &device_creator<k001604_device>;

k001604_device::k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K001604, "Konami 001604", tag, owner, clock, "k001604", __FILE__)
{
	m_token = global_alloc_clear(k001604_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k001604_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k001604_device::device_start()
{
	k001604_state *k001604 = k001604_get_safe_token(this);
	const k001604_interface *intf = k001604_get_interface(this);
	int roz_tile_size;

	k001604->layer_size = intf->layer_size;     // 0 -> width = 128 tiles, 1 -> width = 256 tiles
	k001604->roz_size = intf->roz_size;     // 0 -> 8x8, 1 -> 16x16

	k001604->txt_mem_offset = intf->txt_mem_offset;
	k001604->roz_mem_offset = intf->roz_mem_offset;

	k001604->gfx_index[0] = intf->gfx_index_1;
	k001604->gfx_index[1] = intf->gfx_index_2;

	k001604->char_ram = auto_alloc_array(machine(), UINT32, 0x200000 / 4);
	k001604->tile_ram = auto_alloc_array(machine(), UINT32, 0x20000 / 4);
	k001604->reg = auto_alloc_array(machine(), UINT32, 0x400 / 4);

	/* create tilemaps */
	roz_tile_size = k001604->roz_size ? 16 : 8;

	if (k001604->layer_size)
	{
		k001604->layer_8x8[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_0_size1),this), 8, 8, 64, 64);
		k001604->layer_8x8[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_1_size1),this), 8, 8, 64, 64);

		k001604->layer_roz = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_roz),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_roz_256),this), roz_tile_size, roz_tile_size, 128, 64);
	}
	else
	{
		k001604->layer_8x8[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_0_size0),this), 8, 8, 64, 64);
		k001604->layer_8x8[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_8x8),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_8x8_1_size0),this), 8, 8, 64, 64);

		k001604->layer_roz = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(k001604_device::k001604_tile_info_layer_roz),this), tilemap_mapper_delegate(FUNC(k001604_device::k001604_scan_layer_roz_128),this), roz_tile_size, roz_tile_size, 128, 64);
	}

	k001604->layer_8x8[0]->set_transparent_pen(0);
	k001604->layer_8x8[1]->set_transparent_pen(0);

	machine().gfx[k001604->gfx_index[0]] = auto_alloc(machine(), gfx_element(machine(), k001604_char_layout_layer_8x8, (UINT8*)&k001604->char_ram[0], machine().total_colors() / 16, 0));
	machine().gfx[k001604->gfx_index[1]] = auto_alloc(machine(), gfx_element(machine(), k001604_char_layout_layer_16x16, (UINT8*)&k001604->char_ram[0], machine().total_colors() / 16, 0));

	save_pointer(NAME(k001604->reg), 0x400 / 4);
	save_pointer(NAME(k001604->char_ram), 0x200000 / 4);
	save_pointer(NAME(k001604->tile_ram), 0x20000 / 4);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k001604_device::device_reset()
{
	k001604_state *k001604 = k001604_get_safe_token(this);

	memset(k001604->char_ram, 0, 0x200000);
	memset(k001604->tile_ram, 0, 0x10000);
	memset(k001604->reg, 0, 0x400);
}