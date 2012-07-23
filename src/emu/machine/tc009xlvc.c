/***************************************************************************

	TC009xLVC device emulation

	Written by Angelo Salese, based off Taito L implementation

	TODO:
	- non-video stuff needs to be ported there as well

***************************************************************************/

#include "emu.h"
#include "machine/tc009xlvc.h"

const device_type TC0091LVC = &device_creator<tc0091lvc_device>;


READ8_MEMBER(tc0091lvc_device::tc0091lvc_paletteram_r)
{
	return m_palette_ram[offset & 0x1ff];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_paletteram_w)
{
	m_palette_ram[offset & 0x1ff] = data;

	{
		UINT8 r,g,b,i;
		UINT16 pal;

		pal = (m_palette_ram[offset & ~1]<<0) | (m_palette_ram[offset | 1]<<8);

		i = (pal & 0x7000) >> 12;
		b = (pal & 0x0f00) >> 8;
		g = (pal & 0x00f0) >> 4;
		r = (pal & 0x000f) >> 0;

		r <<= 1;
		g <<= 1;
		b <<= 1;

		/* TODO: correct? */
		b |= ((i & 4) >> 2);
		g |= ((i & 2) >> 1);
		r |= (i & 1);

		palette_set_color_rgb(machine(), offset / 2, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

READ8_MEMBER(tc0091lvc_device::vregs_r)
{
	return m_vregs[offset];
}

WRITE8_MEMBER(tc0091lvc_device::vregs_w)
{
	if((offset & 0xfc) == 0)
	{
		bg0_tilemap->mark_all_dirty();
		bg1_tilemap->mark_all_dirty();
	}

	m_vregs[offset] = data;
}

READ8_MEMBER(tc0091lvc_device::tc0091lvc_bitmap_r)
{
	return m_bitmap_ram[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_bitmap_w)
{
	m_bitmap_ram[offset] = data;
}


READ8_MEMBER(tc0091lvc_device::tc0091lvc_pcg1_r)
{
	return m_pcg1_ram[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_pcg1_w)
{
	m_pcg1_ram[offset] = data;
	gfx_element_mark_dirty(machine().gfx[2], offset / 32);
	tx_tilemap->mark_all_dirty();
}

READ8_MEMBER(tc0091lvc_device::tc0091lvc_pcg2_r)
{
	return m_pcg2_ram[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_pcg2_w)
{
	m_pcg2_ram[offset] = data;
	gfx_element_mark_dirty(machine().gfx[3], offset / 32);
	tx_tilemap->mark_all_dirty();
}

READ8_MEMBER(tc0091lvc_device::tc0091lvc_vram0_r)
{
	return m_vram0[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_vram0_w)
{
	m_vram0[offset] = data;
	bg0_tilemap->mark_tile_dirty(offset/2);
}

READ8_MEMBER(tc0091lvc_device::tc0091lvc_vram1_r)
{
	return m_vram1[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_vram1_w)
{
	m_vram1[offset] = data;
	bg1_tilemap->mark_tile_dirty(offset/2);
}

READ8_MEMBER(tc0091lvc_device::tc0091lvc_tvram_r)
{
	return m_tvram[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_tvram_w)
{
	m_tvram[offset] = data;
	tx_tilemap->mark_tile_dirty(offset/2);
}

READ8_MEMBER(tc0091lvc_device::tc0091lvc_spr_r)
{
	return m_sprram[offset];
}

WRITE8_MEMBER(tc0091lvc_device::tc0091lvc_spr_w)
{
	m_sprram[offset] = data;
}

static ADDRESS_MAP_START( tc0091lvc_map8, AS_0, 8, tc0091lvc_device )
	AM_RANGE(0x014000, 0x017fff) AM_READWRITE(tc0091lvc_pcg1_r, tc0091lvc_pcg1_w)
	AM_RANGE(0x018000, 0x018fff) AM_READWRITE(tc0091lvc_vram0_r, tc0091lvc_vram0_w)
	AM_RANGE(0x019000, 0x019fff) AM_READWRITE(tc0091lvc_vram1_r, tc0091lvc_vram1_w)
	AM_RANGE(0x01a000, 0x01afff) AM_READWRITE(tc0091lvc_tvram_r, tc0091lvc_tvram_w)
	AM_RANGE(0x01b000, 0x01b3ff) AM_READWRITE(tc0091lvc_spr_r, tc0091lvc_spr_w)
	AM_RANGE(0x01b400, 0x01bfff) AM_RAM
	AM_RANGE(0x01c000, 0x01ffff) AM_READWRITE(tc0091lvc_pcg2_r, tc0091lvc_pcg2_w)
	AM_RANGE(0x040000, 0x05ffff) AM_READWRITE(tc0091lvc_bitmap_r, tc0091lvc_bitmap_w)
	AM_RANGE(0x080000, 0x0801ff) AM_READWRITE(tc0091lvc_paletteram_r,tc0091lvc_paletteram_w)
ADDRESS_MAP_END

tc0091lvc_device::tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0091LVC, "TC0091LVC", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  m_space_config("tc0091lvc", ENDIANNESS_LITTLE, 8,20, 0, NULL, *ADDRESS_MAP_NAME(tc0091lvc_map8))
{
}


void tc0091lvc_device::device_config_complete()
{
//  int address_bits = 20;

//  m_space_config = address_space_config("janshi_vdp", ENDIANNESS_LITTLE, 8,  address_bits, 0, *ADDRESS_MAP_NAME(tc0091lvc_map8));
}

void tc0091lvc_device::device_validity_check(validity_checker &valid) const
{
}

static TILE_GET_INFO_DEVICE( get_bg0_tile_info )
{
	tc0091lvc_device *vdp = (tc0091lvc_device*)device;
	int attr = vdp->m_vram0[2 * tile_index + 1];
	int code = vdp->m_vram0[2 * tile_index]
			| ((attr & 0x03) << 8)
			| ((vdp->m_vregs[(attr & 0xc) >> 2]) << 10);
//			| (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO_DEVICE(
			0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO_DEVICE( get_bg1_tile_info )
{
	tc0091lvc_device *vdp = (tc0091lvc_device*)device;
	int attr = vdp->m_vram1[2 * tile_index + 1];
	int code = vdp->m_vram1[2 * tile_index]
			| ((attr & 0x03) << 8)
			| ((vdp->m_vregs[(attr & 0xc) >> 2]) << 10);
//			| (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO_DEVICE(
			0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

static TILE_GET_INFO_DEVICE( get_tx_tile_info )
{
	tc0091lvc_device *vdp = (tc0091lvc_device*)device;
	int attr = vdp->m_tvram[2 * tile_index + 1];
	int code = vdp->m_tvram[2 * tile_index]
			| ((attr & 0x01) << 8);
	int region = ((attr & 0x04) >> 2) + vdp->m_gfx_index;

	SET_TILE_INFO_DEVICE(
			region,
			code,
			(attr & 0xf0) >> 4,
			0);
}


static const gfx_layout char_layout =
{
	8, 8,
	0x4000 / (8 * 4), // need to specify exact number because we create dynamically
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};


void tc0091lvc_device::device_start()
{
	m_palette_ram = auto_alloc_array_clear(machine(), UINT8, 0x200);
	m_vregs = auto_alloc_array_clear(machine(), UINT8, 0x100);
	m_bitmap_ram = auto_alloc_array_clear(machine(), UINT8, 0x20000);
	m_pcg1_ram = auto_alloc_array_clear(machine(), UINT8, 0x4000);
	m_pcg2_ram = auto_alloc_array_clear(machine(), UINT8, 0x4000);
	m_vram0 = auto_alloc_array_clear(machine(), UINT8, 0x1000);
	m_vram1 = auto_alloc_array_clear(machine(), UINT8, 0x1000);
	m_tvram = auto_alloc_array_clear(machine(), UINT8, 0x1000);
	m_sprram = auto_alloc_array_clear(machine(), UINT8, 0x400);
	m_sprram_buffer = auto_alloc_array_clear(machine(), UINT8, 0x400);

	tx_tilemap = tilemap_create_device(this, get_tx_tile_info,tilemap_scan_rows,8,8,64,32);
	bg0_tilemap = tilemap_create_device(this, get_bg0_tile_info,tilemap_scan_rows,8,8,64,32);
	bg1_tilemap = tilemap_create_device(this, get_bg1_tile_info,tilemap_scan_rows,8,8,64,32);

	tx_tilemap->set_transparent_pen(0);
	bg0_tilemap->set_transparent_pen(0);
	bg1_tilemap->set_transparent_pen(0);

	tx_tilemap->set_scrolldx(-8, -8);
	bg0_tilemap->set_scrolldx(28, -11);
	bg1_tilemap->set_scrolldx(38, -21);

	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (machine().gfx[m_gfx_index] == 0)
			break;

	//printf("m_gfx_index %d\n", m_gfx_index);

	machine().gfx[m_gfx_index+0] = gfx_element_alloc(machine(), &char_layout, (UINT8 *)m_pcg1_ram, machine().total_colors() / 16, 0);
	machine().gfx[m_gfx_index+1] = gfx_element_alloc(machine(), &char_layout, (UINT8 *)m_pcg2_ram, machine().total_colors() / 16, 0);
}

void tc0091lvc_device::device_reset()
{

}

const address_space_config *tc0091lvc_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


void tc0091lvc_device::draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 global_flip )
{
	const gfx_element *gfx = machine.gfx[1];
	int count;

	for(count=0;count<0x3e7;count+=8)
	{
		int x,y,spr_offs,col,fx,fy;

		spr_offs = m_sprram_buffer[count+0]|(m_sprram_buffer[count+1]<<8);
		x = m_sprram_buffer[count+4]|(m_sprram_buffer[count+5]<<8);
		if (x >= 320)
			x -= 512;
		y = m_sprram_buffer[count+6];
		col = (m_sprram_buffer[count+2])&0x0f;
		fx = m_sprram_buffer[count+3] & 0x1;
		fy = m_sprram_buffer[count+3] & 0x2;

		if (global_flip)
		{
			x = 304 - x;
			y = 240 - y;
			fx = !fx;
			fy = !fy;
		}

		pdrawgfx_transpen(bitmap,cliprect,gfx,spr_offs,col,fx,fy,x,y,machine.priority_bitmap,(col & 0x08) ? 0xaa : 0x00,0);
	}
}

UINT32 tc0091lvc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT32 count;
	int x,y;
	UINT8 global_flip;

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	if((m_vregs[4] & 0x20) == 0)
		return 0;

	global_flip = m_vregs[4] & 0x10;

	if((m_vregs[4] & 0x8) == 0) // 8bpp bitmap enabled
	{
		count = 0;

		for (y=0;y<256;y++)
		{
			for (x=0;x<512;x++)
			{
				int res_x, res_y;

				res_x = (global_flip) ? 320-x : x;
				res_y = (global_flip) ? 256-y : y;

				if(machine().primary_screen->visible_area().contains(res_x, res_y))
					bitmap.pix16(res_y, res_x) = screen.machine().pens[m_bitmap_ram[count]];

				count++;
			}
		}
	}
	else
	{
		int dx, dy;

		machine().tilemap().set_flip_all(global_flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		dx = m_bg0_scroll[0] | (m_bg0_scroll[1] << 8);
		if (global_flip) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; dx += 192; }
		dy = m_bg0_scroll[2];

		bg0_tilemap->set_scrollx(0, -dx);
		bg0_tilemap->set_scrolly(0, -dy);

		dx = m_bg1_scroll[0] | (m_bg1_scroll[1] << 8);
		if (global_flip) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; dx += 192; }
		dy = m_bg1_scroll[2];

		bg1_tilemap->set_scrollx(0, -dx);
		bg1_tilemap->set_scrolly(0, -dy);

		tx_tilemap->set_scrollx(0, (global_flip) ? -192 : 0);

		machine().priority_bitmap.fill(0, cliprect);
		bg1_tilemap->draw(bitmap, cliprect, 0,0);
		bg0_tilemap->draw(bitmap, cliprect, 0,0);
		draw_sprites(machine(), bitmap, cliprect, global_flip);
		tx_tilemap->draw(bitmap, cliprect, 0,0);
	}
	return 0;
}

void tc0091lvc_device::screen_eof(void)
{
	memcpy(m_sprram_buffer,m_sprram,0x400);
	m_bg0_scroll[0] = m_sprram_buffer[0x3f4];
	m_bg0_scroll[1] = m_sprram_buffer[0x3f5];
	m_bg0_scroll[2] = m_sprram_buffer[0x3f6];

	m_bg1_scroll[0] = m_sprram_buffer[0x3fc];
	m_bg1_scroll[1] = m_sprram_buffer[0x3fd];
	m_bg1_scroll[2] = m_sprram_buffer[0x3fe];
}

