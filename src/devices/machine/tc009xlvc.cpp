// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC009xLVC device emulation

    Written by Angelo Salese, based off Taito L implementation

    TODO:
    - non-video stuff needs to be ported there as well

***************************************************************************/

#include "emu.h"
#include "machine/tc009xlvc.h"

const device_type TC0091LVC = &device_creator<tc0091lvc_device>;


uint8_t tc0091lvc_device::tc0091lvc_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_palette_ram[offset & 0x1ff];
}

void tc0091lvc_device::tc0091lvc_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_palette_ram[offset & 0x1ff] = data;

	{
		uint8_t r,g,b,i;
		uint16_t pal;

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

		m_gfxdecode->palette().set_pen_color(offset / 2, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

uint8_t tc0091lvc_device::vregs_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_vregs[offset];
}

void tc0091lvc_device::vregs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if((offset & 0xfc) == 0)
	{
		bg0_tilemap->mark_all_dirty();
		bg1_tilemap->mark_all_dirty();
	}

	m_vregs[offset] = data;
}

uint8_t tc0091lvc_device::tc0091lvc_bitmap_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_bitmap_ram[offset];
}

void tc0091lvc_device::tc0091lvc_bitmap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_bitmap_ram[offset] = data;
}


uint8_t tc0091lvc_device::tc0091lvc_pcg1_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_pcg1_ram[offset];
}

void tc0091lvc_device::tc0091lvc_pcg1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_pcg1_ram[offset] = data;
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty((offset+0x4000) / 32);
	tx_tilemap->mark_all_dirty();
}

uint8_t tc0091lvc_device::tc0091lvc_pcg2_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_pcg2_ram[offset];
}

void tc0091lvc_device::tc0091lvc_pcg2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_pcg2_ram[offset] = data;
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty((offset+0xc000) / 32);
	tx_tilemap->mark_all_dirty();
}

uint8_t tc0091lvc_device::tc0091lvc_vram0_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_vram0[offset];
}

void tc0091lvc_device::tc0091lvc_vram0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_vram0[offset] = data;
	bg0_tilemap->mark_tile_dirty(offset/2);
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty((offset+0x8000) / 32);
	tx_tilemap->mark_all_dirty();

}

uint8_t tc0091lvc_device::tc0091lvc_vram1_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_vram1[offset];
}

void tc0091lvc_device::tc0091lvc_vram1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_vram1[offset] = data;
	bg1_tilemap->mark_tile_dirty(offset/2);
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty((offset+0x9000) / 32);
	tx_tilemap->mark_all_dirty();
}

uint8_t tc0091lvc_device::tc0091lvc_tvram_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_tvram[offset];
}

void tc0091lvc_device::tc0091lvc_tvram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_tvram[offset] = data;
	tx_tilemap->mark_tile_dirty(offset/2);
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty((offset+0xa000) / 32);
	tx_tilemap->mark_all_dirty();
}

uint8_t tc0091lvc_device::tc0091lvc_spr_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_sprram[offset];
}

void tc0091lvc_device::tc0091lvc_spr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_sprram[offset] = data;
	m_gfxdecode->gfx(m_gfx_index)->mark_dirty((offset+0xb000) / 32);
	tx_tilemap->mark_all_dirty();
}

static ADDRESS_MAP_START( tc0091lvc_map8, AS_0, 8, tc0091lvc_device )
	AM_RANGE(0x014000, 0x017fff) AM_READWRITE(tc0091lvc_pcg1_r, tc0091lvc_pcg1_w)
	AM_RANGE(0x018000, 0x018fff) AM_READWRITE(tc0091lvc_vram0_r, tc0091lvc_vram0_w)
	AM_RANGE(0x019000, 0x019fff) AM_READWRITE(tc0091lvc_vram1_r, tc0091lvc_vram1_w)
	AM_RANGE(0x01a000, 0x01afff) AM_READWRITE(tc0091lvc_tvram_r, tc0091lvc_tvram_w)
	AM_RANGE(0x01b000, 0x01bfff) AM_READWRITE(tc0091lvc_spr_r, tc0091lvc_spr_w)
	AM_RANGE(0x01c000, 0x01ffff) AM_READWRITE(tc0091lvc_pcg2_r, tc0091lvc_pcg2_w)
	AM_RANGE(0x040000, 0x05ffff) AM_READWRITE(tc0091lvc_bitmap_r, tc0091lvc_bitmap_w)
	AM_RANGE(0x080000, 0x0801ff) AM_READWRITE(tc0091lvc_paletteram_r,tc0091lvc_paletteram_w)
ADDRESS_MAP_END

tc0091lvc_device::tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0091LVC, "Taito TC0091LVC", tag, owner, clock, "tc0091lvc", __FILE__)
	, device_memory_interface(mconfig, *this)
	, m_space_config("tc0091lvc", ENDIANNESS_LITTLE, 8,20, 0, nullptr, *ADDRESS_MAP_NAME(tc0091lvc_map8))
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void tc0091lvc_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<tc0091lvc_device &>(device).m_gfxdecode.set_tag(tag);
}


void tc0091lvc_device::device_config_complete()
{
//  int address_bits = 20;

//  m_space_config = address_space_config("janshi_vdp", ENDIANNESS_LITTLE, 8,  address_bits, 0, *ADDRESS_MAP_NAME(tc0091lvc_map8));
}

void tc0091lvc_device::device_validity_check(validity_checker &valid) const
{
}

void tc0091lvc_device::get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_vram0[2 * tile_index + 1];
	int code = m_vram0[2 * tile_index]
			| ((attr & 0x03) << 8)
			| ((m_vregs[(attr & 0xc) >> 2]) << 10);
//          | (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

void tc0091lvc_device::get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_vram1[2 * tile_index + 1];
	int code = m_vram1[2 * tile_index]
			| ((attr & 0x03) << 8)
			| ((m_vregs[(attr & 0xc) >> 2]) << 10);
//          | (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

void tc0091lvc_device::get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_tvram[2 * tile_index + 1];
	uint16_t code = m_tvram[2 * tile_index]
			| ((attr & 0x07) << 8);

	SET_TILE_INFO_MEMBER(m_gfx_index,
			code,
			(attr & 0xf0) >> 4,
			0);
}


static const gfx_layout char_layout =
{
	8, 8,
	0x10000 / (8 * 4), // need to specify exact number because we create dynamically
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};


void tc0091lvc_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	memset(m_palette_ram, 0, ARRAY_LENGTH(m_palette_ram));
	memset(m_vregs, 0, ARRAY_LENGTH(m_vregs));
	memset(m_bitmap_ram, 0, ARRAY_LENGTH(m_bitmap_ram));
	memset(m_pcg_ram, 0, ARRAY_LENGTH(m_pcg_ram));
	memset(m_sprram_buffer, 0, ARRAY_LENGTH(m_sprram_buffer));

	// note, the way tiles are addressed suggests that 0x0000-0x3fff of this might be usable,
	//       but we don't map it anywhere, so the first tiles are always blank at the moment.
	m_pcg1_ram = m_pcg_ram + 0x4000;
	m_pcg2_ram = m_pcg_ram + 0xc000;
	m_vram0 = m_pcg_ram + 0x8000;
	m_vram1 = m_pcg_ram + 0x9000;
	m_tvram = m_pcg_ram + 0xa000;
	m_sprram = m_pcg_ram + 0xb000;

	tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0091lvc_device::get_tx_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	bg0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0091lvc_device::get_bg0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	bg1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0091lvc_device::get_bg1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	tx_tilemap->set_transparent_pen(0);
	bg0_tilemap->set_transparent_pen(0);
	bg1_tilemap->set_transparent_pen(0);

	tx_tilemap->set_scrolldx(-8, -8);
	bg0_tilemap->set_scrolldx(28, -11);
	bg1_tilemap->set_scrolldx(38, -21);

	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (m_gfxdecode->gfx(m_gfx_index) == nullptr)
			break;

	//printf("m_gfx_index %d\n", m_gfx_index);

	palette_device &palette = m_gfxdecode->palette();
	m_gfxdecode->set_gfx(m_gfx_index, std::make_unique<gfx_element>(palette, char_layout, (uint8_t *)m_pcg_ram, 0, palette.entries() / 16, 0));
}

void tc0091lvc_device::device_reset()
{
}

const address_space_config *tc0091lvc_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : nullptr;
}


void tc0091lvc_device::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t global_flip )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
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

		gfx->prio_transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,screen.priority(),(col & 0x08) ? 0xaa : 0x00,0);
	}
}

uint32_t tc0091lvc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t count;
	int x,y;
	uint8_t global_flip;

	bitmap.fill(m_gfxdecode->palette().black_pen(), cliprect);

	if((m_vregs[4] & 0x20) == 0)
		return 0;

	global_flip = m_vregs[4] & 0x10;

	if((m_vregs[4] & 0x7) == 7) // 8bpp bitmap enabled
	{
		count = 0;

		for (y=0;y<256;y++)
		{
			for (x=0;x<512;x++)
			{
				int res_x, res_y;

				res_x = (global_flip) ? 320-x : x;
				res_y = (global_flip) ? 256-y : y;

				if(screen.visible_area().contains(res_x, res_y))
					bitmap.pix16(res_y, res_x) = m_gfxdecode->palette().pen(m_bitmap_ram[count]);

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

		screen.priority().fill(0, cliprect);
		bg1_tilemap->draw(screen, bitmap, cliprect, 0,0);
		bg0_tilemap->draw(screen, bitmap, cliprect, 0,(m_vregs[4] & 0x8) ? 0 : 1);
		draw_sprites(screen, bitmap, cliprect, global_flip);
		tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
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
