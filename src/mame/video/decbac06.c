/*
 Deco BAC06 tilemap generator:

 this a direct relative of the later chip implemented in deco16ic.c
 we could implement this as either an 8-bit or a 16-bit chip, for now
 I'm using the 16-bit implementation from dec0.c

 used by:

 actfancr.c
 dec0.c
 dec8.c (oscar, cobracom, ghostb)
 madmotor.c
 stadhero.c
 pcktgal.c

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

void deco_bac06_device::set_gfx_region_wide(device_t &device, int region8x8, int region16x16, int wide)
{
	deco_bac06_device &dev = downcast<deco_bac06_device &>(device);
	dev.m_gfxregion8x8 = region8x8;
	dev.m_gfxregion16x16 = region16x16;
	dev.m_wide = wide;
}

const device_type DECO_BAC06 = &device_creator<deco_bac06_device>;

deco_bac06_device::deco_bac06_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO_BAC06, "decbac06_device", tag, owner, clock),
	  m_gfxregion8x8(0),
	  m_gfxregion16x16(0),
	  m_wide(0)
{
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape0_scan)
{
	return (col & 0xf) + ((row & 0xf) << 4) + ((col & 0x1f0) << 4);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape1_scan)
{
	return (col & 0xf) + ((row & 0x1f) << 4) + ((col & 0xf0) << 5);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape2_scan)
{
	return (col & 0xf) + ((row & 0x3f) << 4) + ((col & 0x70) << 6);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape0_8x8_scan)
{
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape1_8x8_scan)
{
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x20) << 5) + ((col & 0x20) << 6);
}

TILEMAP_MAPPER_MEMBER(deco_bac06_device::tile_shape2_8x8_scan)
{
	return (col & 0x1f) + ((row & 0x7f) << 5);
}

TILE_GET_INFO_MEMBER(deco_bac06_device::get_pf8x8_tile_info)
{
	if (m_rambank&1) tile_index+=0x1000;
	int tile=pf_data[tile_index];
	int colourpri=(tile>>12);
	SET_TILE_INFO_MEMBER(tile_region,tile&0xfff,0,0);
	tileinfo.category = colourpri;
}

TILE_GET_INFO_MEMBER(deco_bac06_device::get_pf16x16_tile_info)
{
	if (m_rambank&1) tile_index+=0x1000;
	int tile=pf_data[tile_index];
	int colourpri=(tile>>12);
	SET_TILE_INFO_MEMBER(tile_region,tile&0xfff,0,0);
	tileinfo.category = colourpri;
}

void deco_bac06_device::create_tilemaps(int region8x8, int region16x16)
{
	tile_region = region8x8;

	pf8x8_tilemap[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf8x8_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_8x8_scan),this), 8, 8,128, 32);
	pf8x8_tilemap[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf8x8_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_8x8_scan),this), 8, 8, 64, 64);
	pf8x8_tilemap[2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf8x8_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_8x8_scan),this), 8, 8, 32,128);

	tile_region = region16x16;

	if (m_wide==2)
	{
		pf16x16_tilemap[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_scan),this), 16, 16, 256, 16);
		pf16x16_tilemap[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_scan),this),  16, 16,  128, 32);
		pf16x16_tilemap[2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_scan),this),  16, 16,  64, 64);
	}
	else if (m_wide==1)
	{
		pf16x16_tilemap[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_scan),this), 16, 16, 128, 16);
		pf16x16_tilemap[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_scan),this),  16, 16,  64, 32);
		pf16x16_tilemap[2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this), tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_scan),this),  16, 16,  32, 64);
	}
	else
	{
		pf16x16_tilemap[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape0_scan),this),    16,16, 64, 16);
		pf16x16_tilemap[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape1_scan),this),    16,16, 32, 32);
		pf16x16_tilemap[2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(deco_bac06_device::get_pf16x16_tile_info),this),tilemap_mapper_delegate(FUNC(deco_bac06_device::tile_shape2_scan),this),    16,16, 16, 64);
	}
}

void deco_bac06_device::device_start()
{
	pf_data = auto_alloc_array_clear(this->machine(), UINT16, 0x4000 / 2); // 0x2000 is the maximum needed, some games / chip setups map less and mirror - stadium hero banks this to 0x4000?!
	pf_rowscroll = auto_alloc_array_clear(this->machine(), UINT16, 0x2000 / 2);
	pf_colscroll = auto_alloc_array_clear(this->machine(), UINT16, 0x2000 / 2);

	create_tilemaps(m_gfxregion8x8, m_gfxregion16x16);
	m_gfxcolmask = 0x0f;

	m_bppmult = 0x10;
	m_bppmask = 0x0f;
	m_rambank = 0;
}

void deco_bac06_device::device_reset()
{

}


void deco_bac06_device::custom_tilemap_draw(running_machine &machine,
								bitmap_ind16 &bitmap,
								const rectangle &cliprect,
								tilemap_t *tilemap_ptr,
								const UINT16 *rowscroll_ptr,
								const UINT16 *colscroll_ptr,
								const UINT16 *control0,
								const UINT16 *control1,
								int flags,
								UINT16 penmask,
								UINT16 pencondition,
								UINT16 colprimask,
								UINT16 colpricondition
								)
{
	const bitmap_ind16 &src_bitmap = tilemap_ptr->pixmap();
	const bitmap_ind8 &flags_bitmap = tilemap_ptr->flagsmap();
	int x, y, p, colpri;
	int column_offset=0, src_x=0, src_y=0;
	UINT32 scrollx = 0;
	UINT32 scrolly = 0;

	if (control1)
	{
		scrollx = control1[0];
		scrolly = control1[1];
	}

	int width_mask;
	int height_mask;
	int row_scroll_enabled = 0;
	int col_scroll_enabled = 0;

	if (control0)
	{
		row_scroll_enabled = (rowscroll_ptr && (control0[0]&0x4));
		col_scroll_enabled = (colscroll_ptr && (control0[0]&0x8));
	}

	width_mask = src_bitmap.width() - 1;
	height_mask = src_bitmap.height() - 1;

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

	if (machine.driver_data()->flip_screen())
		src_y = (src_bitmap.height() - 256) - scrolly;
	else
		src_y = scrolly;

	for (y=0; y<=cliprect.max_y; y++) {
		if (row_scroll_enabled)
			src_x=scrollx + rowscroll_ptr[(src_y >> (control1[3]&0xf))&(0x1ff>>(control1[3]&0xf))];
		else
			src_x=scrollx;

		if (machine.driver_data()->flip_screen())
			src_x=(src_bitmap.width() - 256) - src_x;

		for (x=0; x<=cliprect.max_x; x++) {
			if (col_scroll_enabled)
				column_offset=colscroll_ptr[((src_x >> 3) >> (control1[2]&0xf))&(0x3f>>(control1[2]&0xf))];

			p = src_bitmap.pix16((src_y + column_offset)&height_mask, src_x&width_mask);
			colpri =  flags_bitmap.pix8((src_y + column_offset)&height_mask, src_x&width_mask)&0xf;

			src_x++;
			if ((flags&TILEMAP_DRAW_OPAQUE) || (p&m_bppmask))
			{


				if ((p&penmask)==pencondition)
					if((colpri&colprimask)==colpricondition)
						bitmap.pix16(y, x) = p+(colpri&m_gfxcolmask)*m_bppmult;
			}
		}
		src_y++;
	}
}

void deco_bac06_device::deco_bac06_pf_draw(running_machine &machine,bitmap_ind16 &bitmap,const rectangle &cliprect,int flags,UINT16 penmask, UINT16 pencondition,UINT16 colprimask, UINT16 colpricondition)
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
		custom_tilemap_draw(machine,bitmap,cliprect,tm,pf_rowscroll,pf_colscroll,pf_control_0,pf_control_1,flags, penmask, pencondition, colprimask, colpricondition);

}

// used for pocket gal bootleg, which doesn't set registers properly and simply expects a fixed size tilemap.
void deco_bac06_device::deco_bac06_pf_draw_bootleg(running_machine &machine,bitmap_ind16 &bitmap,const rectangle &cliprect,int flags, int mode, int type)
{
	tilemap_t* tm = 0;
	if (!mode) tm = pf8x8_tilemap[type];
	else tm = pf16x16_tilemap[type];

	custom_tilemap_draw(machine,bitmap,cliprect,tm,pf_rowscroll,pf_colscroll,0,0,flags, 0, 0, 0, 0);
}



WRITE16_DEVICE_HANDLER( deco_bac06_pf_control_0_w )
{
	offset &= 3;
	deco_bac06_device *dev = (deco_bac06_device*)device;

	COMBINE_DATA(&dev->pf_control_0[offset]);

	if (offset==2)
	{
		int newbank = dev->pf_control_0[offset]&1;
		if ((newbank&1) != (dev->m_rambank&1))
		{
			// I don't know WHY Stadium Hero uses this as a bank but the RAM test expects it..
			// I'm curious as to if anything else sets it tho
			if (strcmp(dev->machine().system().name,"stadhero"))
				printf("tilemap ram bank change to %d\n", newbank&1);

			dev->m_rambank = newbank&1;
			dev->pf8x8_tilemap[0]->mark_all_dirty();
			dev->pf8x8_tilemap[1]->mark_all_dirty();
			dev->pf8x8_tilemap[2]->mark_all_dirty();
			dev->pf16x16_tilemap[0]->mark_all_dirty();
			dev->pf16x16_tilemap[1]->mark_all_dirty();
			dev->pf16x16_tilemap[2]->mark_all_dirty();
		}
	}
}

READ16_DEVICE_HANDLER( deco_bac06_pf_control_1_r )
{
	offset &= 7;
	deco_bac06_device *dev = (deco_bac06_device*)device;
	return dev->pf_control_1[offset];
}

WRITE16_DEVICE_HANDLER( deco_bac06_pf_control_1_w )
{
	offset &= 7;
	deco_bac06_device *dev = (deco_bac06_device*)device;
	COMBINE_DATA(&dev->pf_control_1[offset]);
}

WRITE16_DEVICE_HANDLER( deco_bac06_pf_data_w )
{

	deco_bac06_device *dev = (deco_bac06_device*)device;
	if (dev->m_rambank&1) offset+=0x1000;

	COMBINE_DATA(&dev->pf_data[offset]);
	dev->pf8x8_tilemap[0]->mark_tile_dirty(offset);
	dev->pf8x8_tilemap[1]->mark_tile_dirty(offset);
	dev->pf8x8_tilemap[2]->mark_tile_dirty(offset);
	dev->pf16x16_tilemap[0]->mark_tile_dirty(offset);
	dev->pf16x16_tilemap[1]->mark_tile_dirty(offset);
	dev->pf16x16_tilemap[2]->mark_tile_dirty(offset);
}

READ16_DEVICE_HANDLER( deco_bac06_pf_data_r )
{
	deco_bac06_device *dev = (deco_bac06_device*)device;
	if (dev->m_rambank&1) offset+=0x1000;

	return dev->pf_data[offset];
}



WRITE8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_w )
{
	if (offset&1)
		deco_bac06_pf_data_w(device,space,offset/2,data,0x00ff);
	else
		deco_bac06_pf_data_w(device,space,offset/2,data<<8,0xff00);
}

READ8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_r )
{
	if (offset&1) /* MSB */
		return deco_bac06_pf_data_r(device,space,offset/2,0x00ff);
	else
		return deco_bac06_pf_data_r(device,space,offset/2,0xff00)>>8;
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

/* used by dec8.c */
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control0_8bit_w )
{
	if (offset&1)
		deco_bac06_pf_control_0_w(device,space,offset/2,data,0x00ff); // oscar (mirrors?)
	else
		deco_bac06_pf_control_0_w(device,space,offset/2,data,0x00ff);
}

/* used by dec8.c */
READ8_DEVICE_HANDLER( deco_bac06_pf_control1_8bit_r )
{
	if (offset&1)
		return deco_bac06_pf_control_1_r(device,space,offset/2,0x00ff);
	else
		return deco_bac06_pf_control_1_r(device,space,offset/2,0xff00)>>8;
}

/* used by dec8.c */
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control1_8bit_w )
{
	if (offset<4) // these registers are 16-bit?
	{
		if (offset&1)
			deco_bac06_pf_control_1_w(device,space,offset/2,data,0x00ff);
		else
			deco_bac06_pf_control_1_w(device,space,offset/2,data<<8,0xff00);
	}
	else // these registers are 8-bit and mirror? (triothep vs actfancr)
	{
		if (offset&1)
			deco_bac06_pf_control_1_w(device,space,offset/2,data,0x00ff);
		else
			deco_bac06_pf_control_1_w(device,space,offset/2,data,0x00ff);
	}
}

READ8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_r )
{
	if (offset&1)
		return deco_bac06_pf_rowscroll_r(device,space,offset/2,0x00ff);
	else
		return deco_bac06_pf_rowscroll_r(device,space,offset/2,0xff00)>>8;
}


WRITE8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_w )
{
	if (offset&1)
		deco_bac06_pf_rowscroll_w(device,space,offset/2,data,0x00ff);
	else
		deco_bac06_pf_rowscroll_w(device,space,offset/2,data<<8,0xff00);
}

READ8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_swap_r )
{
	if (offset&1)
		return deco_bac06_pf_rowscroll_r(device,space,offset/2,0xff00)>>8;
	else
		return deco_bac06_pf_rowscroll_r(device,space,offset/2,0x00ff);
}

WRITE8_DEVICE_HANDLER( deco_bac06_pf_rowscroll_8bit_swap_w )
{
	if (offset&1)
		deco_bac06_pf_rowscroll_w(device,space,offset/2,data<<8,0xff00);
	else
		deco_bac06_pf_rowscroll_w(device,space,offset/2,data,0x00ff);
}



/* used by hippodrm */
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control0_8bit_packed_w )
{
	if (offset&1)
		deco_bac06_pf_control_0_w(device,space,offset/2,data<<8,0xff00);
	else
		deco_bac06_pf_control_0_w(device,space,offset/2,data,0x00ff);
}

/* used by hippodrm */
WRITE8_DEVICE_HANDLER( deco_bac06_pf_control1_8bit_swap_w )
{
	deco_bac06_pf_control1_8bit_w(device,space, offset^1, data);
}

/* used by hippodrm */
READ8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_swap_r )
{
	return deco_bac06_pf_data_8bit_r(device,space, offset^1);
}

/* used by hippodrm */
WRITE8_DEVICE_HANDLER( deco_bac06_pf_data_8bit_swap_w )
{
	deco_bac06_pf_data_8bit_w(device,space, offset^1, data);
}
