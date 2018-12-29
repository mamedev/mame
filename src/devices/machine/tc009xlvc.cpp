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

#include "screen.h"
#include "emupal.h"


DEFINE_DEVICE_TYPE(TC0091LVC, tc0091lvc_device, "tc009xlvc", "Taito TC0091LVC")


READ8_MEMBER(tc0091lvc_device::vregs_r)
{
	return m_vregs[offset];
}

WRITE8_MEMBER(tc0091lvc_device::vregs_w)
{
	if((offset & 0xfc) == 0)
	{
		m_bg_tilemap[0]->mark_all_dirty();
		m_bg_tilemap[1]->mark_all_dirty();
	}

	m_vregs[offset] = data;
}

WRITE8_MEMBER(tc0091lvc_device::vram_w)
{
	// TODO : 0x10000-0x13fff Unused?
	if ((offset & 0xc000) == 0)
		return;

	assert(m_vram[offset] != data);

	COMBINE_DATA(&m_vram[offset]);
	gfx(2)->mark_dirty(offset / 32);

	if ((offset >= 0x8000) && (offset < 0xa000)) // 0x8000-0x9000 Background tilemap
		m_bg_tilemap[(offset >> 12) & 1]->mark_tile_dirty((offset & 0xfff) / 2);
	else if (offset < 0xb000) // 0xa000 Text tilemap
		m_tx_tilemap->mark_tile_dirty((offset & 0xfff) / 2);

}

void tc0091lvc_device::tc0091lvc_map8(address_map &map)
{
	map(0x010000, 0x01ffff).ram().w(FUNC(tc0091lvc_device::vram_w)).share("vram");
	map(0x040000, 0x05ffff).ram().share("bitmap_ram");
	map(0x080000, 0x0801ff).mirror(0x00e00).ram().w("palette", FUNC(palette_device::write8)).share("palette"); // mirrored?
}

static const gfx_layout bg2_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16 },
	{ STEP8(0,8*4) },
	8*8*4
};


#define O 8*8*4
#define O2 2*O
static const gfx_layout sp2_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16, O+3, O+2, O+1, O+0, O+19, O+18, O+17, O+16 },
	{ STEP8(0,8*4), STEP8(O2,8*4) },
	8*8*4*4
};
#undef O
#undef O2

static const gfx_layout char_layout =
{
	8, 8,
	0x10000 / (8 * 4), // need to specify exact number because we create dynamically
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ STEP8(0,8*4) },
	8*8*4
};

static GFXDECODE_START( gfx_tc0091lvc )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, bg2_layout,  0, 16 )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, sp2_layout,  0, 16 )
	GFXDECODE_DEVICE_RAM( "vram",  0, char_layout, 0, 16 )  // Ram-based
GFXDECODE_END

tc0091lvc_device::tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0091LVC, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfx_tc0091lvc, "palette")
	, device_memory_interface(mconfig, *this)
	, m_space_config("tc0091lvc", ENDIANNESS_LITTLE, 8,20, 0, address_map_constructor(), address_map_constructor(FUNC(tc0091lvc_device::tc0091lvc_map8), this))
	, m_vram(*this, "vram")
	, m_bitmap_ram(*this, "bitmap_ram")
{
}

template<int Offset>
TILE_GET_INFO_MEMBER(tc0091lvc_device::get_bg_tile_info)
{
	int attr = m_vram[2 * tile_index + Offset + 1];
	int code = m_vram[2 * tile_index + Offset]
			| ((attr & 0x03) << 8)
			| ((m_vregs[(attr & 0xc) >> 2]) << 10);
//          | (state->m_horshoes_gfxbank << 12);

	SET_TILE_INFO_MEMBER(0,
			code,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(tc0091lvc_device::get_tx_tile_info)
{
	int attr = m_vram[2 * tile_index + 0xa000 + 1];
	int code = m_vram[2 * tile_index + 0xa000]
			| ((attr & 0x07) << 8);

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0xf0) >> 4,
			0);
}


void tc0091lvc_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, "palette", palette_device::BLACK).set_format(2, &tc0091lvc_device::tc0091lvc_xBGRBBBBGGGGRRRR, 256);
}

void tc0091lvc_device::device_start()
{
	m_vregs = make_unique_clear<uint8_t[]>(0x100);
	m_sprram_buffer = make_unique_clear<uint8_t[]>(0x400);

	std::fill_n(&m_bitmap_ram[0], m_bitmap_ram.bytes(), 0);
	std::fill_n(&m_vram[0], m_vram.bytes(), 0);
	// note, the way tiles are addressed suggests that 0x0000-0x3fff of this might be usable,
	//       but we don't map it anywhere, so the first tiles are always blank at the moment.

	m_tx_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(tc0091lvc_device::get_tx_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(tc0091lvc_device::get_bg_tile_info<0x8000>),this),TILEMAP_SCAN_ROWS,8,8,64,32);
	m_bg_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(FUNC(tc0091lvc_device::get_bg_tile_info<0x9000>),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_tx_tilemap->set_transparent_pen(0);
	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);

	m_tx_tilemap->set_scrolldx(-8, -8);
	m_bg_tilemap[0]->set_scrolldx(28, -11);
	m_bg_tilemap[1]->set_scrolldx(38, -21);

	save_pointer(NAME(m_vregs), 0x400);
	save_pointer(NAME(m_sprram_buffer), 0x400);
}

device_memory_interface::space_config_vector tc0091lvc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


rgb_t tc0091lvc_device::tc0091lvc_xBGRBBBBGGGGRRRR(uint32_t raw)
{
	/* TODO: correct? */
	u8 const r = ((raw & 0x00f) << 1) | ((raw & 0x1000) >> 12);
	u8 const g = ((raw & 0x0f0) >> 3) | ((raw & 0x2000) >> 13);
	u8 const b = ((raw & 0xf00) >> 7) | ((raw & 0x4000) >> 14);
	return rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
}

void tc0091lvc_device::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t global_flip )
{
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

		gfx(1)->prio_transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,screen.priority(),(col & 0x08) ? 0xaa : 0x00,0);
	}
}

uint32_t tc0091lvc_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint32_t count;
	int x,y;
	uint8_t global_flip;

	bitmap.fill(palette().black_pen(), cliprect);

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
					bitmap.pix16(res_y, res_x) = palette().pen(m_bitmap_ram[count]);

				count++;
			}
		}
	}
	else
	{
		int dx, dy;

		machine().tilemap().set_flip_all(global_flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		dx = m_bg_scroll[0][0] | (m_bg_scroll[0][1] << 8);
		if (global_flip) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; dx += 192; }
		dy = m_bg_scroll[0][2];

		m_bg_tilemap[0]->set_scrollx(0, -dx);
		m_bg_tilemap[0]->set_scrolly(0, -dy);

		dx = m_bg_scroll[1][0] | (m_bg_scroll[1][1] << 8);
		if (global_flip) { dx = ((dx & 0xfffc) | ((dx - 3) & 0x0003)) ^ 0xf; dx += 192; }
		dy = m_bg_scroll[1][2];

		m_bg_tilemap[1]->set_scrollx(0, -dx);
		m_bg_tilemap[1]->set_scrolly(0, -dy);

		m_tx_tilemap->set_scrollx(0, (global_flip) ? -192 : 0);

		screen.priority().fill(0, cliprect);
		m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0,0);
		m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0,(m_vregs[4] & 0x8) ? 0 : 1);
		draw_sprites(screen, bitmap, cliprect, global_flip);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	}
	return 0;
}

WRITE_LINE_MEMBER(tc0091lvc_device::screen_vblank)
{
	if (state)
	{
		std::copy_n(&m_vram[0xb000], 0x400, &m_sprram_buffer[0]);
		m_bg_scroll[0][0] = m_sprram_buffer[0x3f4];
		m_bg_scroll[0][1] = m_sprram_buffer[0x3f5];
		m_bg_scroll[0][2] = m_sprram_buffer[0x3f6];

		m_bg_scroll[1][0] = m_sprram_buffer[0x3fc];
		m_bg_scroll[1][1] = m_sprram_buffer[0x3fd];
		m_bg_scroll[1][2] = m_sprram_buffer[0x3fe];
	}
}
