/*
 Deco BAC06 tilemap generator:

 this a direct relative of the later chip implemented in deco16ic.c
 we could implement this as either an 8-bit or a 16-bit chip, for now
 I'm using the 16-bit implementation from dec0.c



 Notes (from dec0.c)

 All games contain three BAC06 background generator chips, usual (software)
configuration is 2 chips of 16*16 tiles, 1 of 8*8.

 Playfield control registers:
   bank 0:
   0:
        bit 0 (0x1) set = 8*8 tiles, else 16*16 tiles
        Bit 1 (0x2) unknown
        bit 2 (0x4) set enables rowscroll
        bit 3 (0x8) set enables colscroll
        bit 7 (0x80) set in playfield 1 is reverse screen (set via dip-switch)
        bit 7 (0x80) in other playfields unknown
   2: unknown (00 in bg, 03 in fg+text - maybe controls pf transparency?)
   4: unknown (always 00) [Used to access 2nd bank of tiles in Stadium Hero)
   6: playfield shape: 00 = 4x1, 01 = 2x2, 02 = 1x4 (low 4 bits only)

   bank 1:
   0: horizontal scroll
   2: vertical scroll
   4: colscroll shifter (low 4 bits, top 4 bits do nothing)
   6: rowscroll shifter (low 4 bits, top 4 bits do nothing)

   Row & column scroll can be applied simultaneously or by themselves.
   The shift register controls the granularity of the scroll offsets
   (more details given later).

Playfield priority (Bad Dudes, etc):
    In the bottommost playfield, pens 8-15 can have priority over the next playfield.
    In that next playfield, pens 8-15 can have priority over sprites.

Bit 0:  Playfield inversion
Bit 1:  Enable playfield mixing (for palettes 8-15 only)
Bit 2:  Enable playfield/sprite mixing (for palettes 8-15 only)

Priority word (Midres):
    Bit 0 set = Playfield 3 drawn over Playfield 2
            ~ = Playfield 2 drawn over Playfield 3
    Bit 1 set = Sprites are drawn inbetween playfields
            ~ = Sprites are on top of playfields
    Bit 2
    Bit 3 set = ...

*/

#include "emu.h"
#include "decbac06.h"

deco_bac06_device_config::deco_bac06_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "decbac06_device", tag, owner, clock)
{
	m_gfxregion8x8 = m_gfxregion16x16 = 0;
}

device_config *deco_bac06_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(deco_bac06_device_config(mconfig, tag, owner, clock));
}

device_t *deco_bac06_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, deco_bac06_device(machine, *this));
}

void deco_bac06_device_config::set_gfx_region(device_config *device, int region8x8, int region16x16)
{
	deco_bac06_device_config *dev = downcast<deco_bac06_device_config *>(device);
	dev->m_gfxregion8x8 = region8x8;
	dev->m_gfxregion16x16 = region16x16;
}


deco_bac06_device::deco_bac06_device(running_machine &_machine, const deco_bac06_device_config &config)
	: device_t(_machine, config),
	  m_config(config),
	  m_gfxregion8x8(m_config.m_gfxregion8x8),
	  m_gfxregion16x16(m_config.m_gfxregion16x16)
{
}



static TILEMAP_MAPPER( tile_shape0_scan )
{
	return (col & 0xf) + ((row & 0xf) << 4) + ((col & 0x30) << 4);
}

static TILEMAP_MAPPER( tile_shape1_scan )
{
	return (col & 0xf) + ((row & 0xf) << 4) + ((row & 0x10) << 4) + ((col & 0x10) << 5);
}

static TILEMAP_MAPPER( tile_shape2_scan )
{
	return (col & 0xf) + ((row & 0x3f) << 4);
}

static TILEMAP_MAPPER( tile_shape0_8x8_scan )
{
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

static TILEMAP_MAPPER( tile_shape1_8x8_scan )
{
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x20) << 5) + ((col & 0x20) << 6);
}

static TILEMAP_MAPPER( tile_shape2_8x8_scan )
{
	return (col & 0x1f) + ((row & 0x7f) << 5);
}

static TILE_GET_INFO_DEVICE( get_pf8x8_tile_info )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	int tile=dev->pf_data[tile_index];
	SET_TILE_INFO_DEVICE(dev->tile_region,tile&0xfff,tile>>12,0);
}

static TILE_GET_INFO_DEVICE( get_pf16x16_tile_info )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	int tile=dev->pf_data[tile_index];
	int pri=((tile>>12)>7);
	SET_TILE_INFO_DEVICE(dev->tile_region,tile&0xfff,tile>>12,0);
	tileinfo->group = pri;
}


void deco_bac06_device::create_tilemaps(int region8x8, int region16x16)
{
	tile_region = region8x8;

	pf8x8_tilemap[0] = tilemap_create_device(this, get_pf8x8_tile_info,tile_shape0_8x8_scan, 8, 8,128, 32);
	pf8x8_tilemap[1] = tilemap_create_device(this, get_pf8x8_tile_info,tile_shape1_8x8_scan, 8, 8, 64, 64);
	pf8x8_tilemap[2] = tilemap_create_device(this, get_pf8x8_tile_info,tile_shape2_8x8_scan, 8, 8, 32,128);

	tile_region = region16x16;

	pf16x16_tilemap[0] = tilemap_create_device(this, get_pf16x16_tile_info,tile_shape0_scan,    16,16, 64, 16);
	pf16x16_tilemap[1] = tilemap_create_device(this, get_pf16x16_tile_info,tile_shape1_scan,    16,16, 32, 32);
	pf16x16_tilemap[2] = tilemap_create_device(this, get_pf16x16_tile_info,tile_shape2_scan,    16,16, 16, 64);
}

void deco_bac06_device::device_start()
{
	pf_data = auto_alloc_array_clear(this->machine, UINT16, 0x2000 / 2); // 0x2000 is the maximum needed, some games / chip setups map less and mirror
	pf_rowscroll = auto_alloc_array_clear(this->machine, UINT16, 0x2000 / 2);
	pf_colscroll = auto_alloc_array_clear(this->machine, UINT16, 0x2000 / 2);

	create_tilemaps(m_gfxregion8x8, m_gfxregion16x16);	
}

void deco_bac06_device::device_reset()
{

}


static void custom_tilemap_draw(running_machine *machine,
								bitmap_t *bitmap,
								const rectangle *cliprect,
								tilemap_t *tilemap_ptr,
								const UINT16 *rowscroll_ptr,
								const UINT16 *colscroll_ptr,
								const UINT16 *control0,
								const UINT16 *control1,
								int flags)
{
	const bitmap_t *src_bitmap = tilemap_get_pixmap(tilemap_ptr);
	int x, y, p;
	int column_offset=0, src_x=0, src_y=0;
	UINT32 scrollx=control1[0];
	UINT32 scrolly=control1[1];
	int width_mask;
	int height_mask;
	int row_scroll_enabled = (rowscroll_ptr && (control0[0]&0x4));
	int col_scroll_enabled = (colscroll_ptr && (control0[0]&0x8));

	if (!src_bitmap)
		return;

	width_mask = src_bitmap->width - 1;
	height_mask = src_bitmap->height - 1;

	/* Column scroll & row scroll may per applied per pixel, there are
    shift registers for each which control the granularity of the row/col
    offset (down to per line level for row, and per 8 lines for column).

    Nb:  The row & col selectors are _not_ affected by the shape of the
    playfield (ie, 256*1024, 512*512 or 1024*256).  So even if the tilemap
    width is only 256, 'src_x' should not wrap at 256 in the code below (to
    do so would mean the top half of row RAM would never be accessed which
    is incorrect).

    Nb2:  Real hardware exhibits a strange bug with column scroll on 'mode 2'
    (256*1024) - the first column has a strange additional offset, but
    curiously the first 'wrap' (at scroll offset 256) does not have this offset,
    it is displayed as expected.  The bug is confimed to only affect this mode,
    the other two modes work as expected.  This bug is not emulated, as it
    doesn't affect any games.
    */

	if (flip_screen_get(machine))
		src_y = (src_bitmap->height - 256) - scrolly;
	else
		src_y = scrolly;

	for (y=0; y<=cliprect->max_y; y++) {
		if (row_scroll_enabled)
			src_x=scrollx + rowscroll_ptr[(src_y >> (control1[3]&0xf))&(0x1ff>>(control1[3]&0xf))];
		else
			src_x=scrollx;

		if (flip_screen_get(machine))
			src_x=(src_bitmap->width - 256) - src_x;

		for (x=0; x<=cliprect->max_x; x++) {
			if (col_scroll_enabled)
				column_offset=colscroll_ptr[((src_x >> 3) >> (control1[2]&0xf))&(0x3f>>(control1[2]&0xf))];

			p = *BITMAP_ADDR16(src_bitmap, (src_y + column_offset)&height_mask, src_x&width_mask);

			src_x++;
			if ((flags&TILEMAP_DRAW_OPAQUE) || (p&0xf))
			{
				if( flags & TILEMAP_DRAW_LAYER0 )
				{
					/* Top 8 pens of top 8 palettes only */
					if ((p&0x88)==0x88)
						*BITMAP_ADDR16(bitmap, y, x) = p;
				}
				else
					*BITMAP_ADDR16(bitmap, y, x) = p;
			}
		}
		src_y++;
	}
}


void deco_bac06_device::deco_bac06_pf_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int flags)
{
	tilemap_t* tm = 0;

	switch (pf_control_0[3]&0x3) {
		case 0:	/* 4x1 */
			if (pf_control_0[0]&0x1)
				tm = pf8x8_tilemap[0];
			else
				tm = pf16x16_tilemap[0];
			break;

		case 1:	/* 2x2 */
		default:
			if (pf_control_0[0]&0x1)
				tm = pf8x8_tilemap[1];
			else
				tm = pf16x16_tilemap[1];
			break;

		case 2:	/* 1x4 */
			if (pf_control_0[0]&0x1)
				tm = pf8x8_tilemap[2];
			else
				tm = pf16x16_tilemap[2];
			break;
	};

	if (tm)
		custom_tilemap_draw(machine,bitmap,cliprect,tm,pf_rowscroll,pf_colscroll,pf_control_0,pf_control_1,flags);

}



WRITE16_DEVICE_HANDLER( deco_bac06_pf_control_0_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	COMBINE_DATA(&dev->pf_control_0[offset]);
}

WRITE16_DEVICE_HANDLER( deco_bac06_pf_control_1_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	COMBINE_DATA(&dev->pf_control_1[offset]);
}

WRITE16_DEVICE_HANDLER( deco_bac06_pf_data_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	COMBINE_DATA(&dev->pf_data[offset]);
	tilemap_mark_tile_dirty(dev->pf8x8_tilemap[0],offset);
	tilemap_mark_tile_dirty(dev->pf8x8_tilemap[1],offset);
	tilemap_mark_tile_dirty(dev->pf8x8_tilemap[2],offset);
	tilemap_mark_tile_dirty(dev->pf16x16_tilemap[0],offset);
	tilemap_mark_tile_dirty(dev->pf16x16_tilemap[1],offset);
	tilemap_mark_tile_dirty(dev->pf16x16_tilemap[2],offset);
}

READ16_DEVICE_HANDLER( deco_bac06_pf_data_r )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	return dev->pf_data[offset];
}

WRITE8_DEVICE_HANDLER( deco_bac06_pf_control_8bit_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	UINT16 myword;

	dev->buffer[offset]=data;

	/* Rearrange little endian bytes from H6280 into big endian words for 68k */
	offset&=0xffe;
	myword=dev->buffer[offset] + (dev->buffer[offset+1]<<8);

	if (offset<0x10) deco_bac06_pf_control_0_w(device,offset/2,myword,0xffff);
	else deco_bac06_pf_control_1_w(device,(offset-0x10)/2,myword,0xffff);
}

WRITE8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	if (offset&1) { /* MSB has changed */
		UINT16 lsb=dev->pf_data[offset>>1];
		UINT16 newword=(lsb&0xff) | (data<<8);
		dev->pf_data[offset>>1]=newword;
	}
	else { /* LSB has changed */
		UINT16 msb=dev->pf_data[offset>>1];
		UINT16 newword=(msb&0xff00) | data;
		dev->pf_data[offset>>1]=newword;
	}
	tilemap_mark_tile_dirty(dev->pf8x8_tilemap[0],offset>>1);
	tilemap_mark_tile_dirty(dev->pf8x8_tilemap[1],offset>>1);
	tilemap_mark_tile_dirty(dev->pf8x8_tilemap[2],offset>>1);
	tilemap_mark_tile_dirty(dev->pf16x16_tilemap[0],offset>>1);
	tilemap_mark_tile_dirty(dev->pf16x16_tilemap[1],offset>>1);
	tilemap_mark_tile_dirty(dev->pf16x16_tilemap[2],offset>>1);
}

READ8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_r )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	if (offset&1) /* MSB */
		return dev->pf_data[offset>>1]>>8;

	return dev->pf_data[offset>>1]&0xff;
}

WRITE16_DEVICE_HANDLER( deco_bac06_pf_rowscroll_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	COMBINE_DATA(&dev->pf_rowscroll[offset]);
}

WRITE16_DEVICE_HANDLER( deco_bac06_pf_colscroll_w )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	COMBINE_DATA(&dev->pf_colscroll[offset]);
}

READ16_DEVICE_HANDLER( deco_bac06_pf_rowscroll_r )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	return dev->pf_rowscroll[offset];
}

READ16_DEVICE_HANDLER( deco_bac06_pf_colscroll_r )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	return dev->pf_colscroll[offset];
}

