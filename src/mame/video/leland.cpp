// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cinemat/Leland driver

    Leland video hardware

***************************************************************************/

#include "emu.h"
#include "includes/leland.h"
#include "audio/leland.h"


/* constants */
static constexpr int VRAM_SIZE = 0x10000;
static constexpr int QRAM_SIZE = 0x10000;


/* debugging */
#define LOG_COMM    0

/*************************************
 *
 *  Scanline callback
 *
 *************************************/

TIMER_CALLBACK_MEMBER(leland_state::scanline_callback)
{
	int scanline = param;

	/* update the DACs */
	if (!(m_dac_control & 0x01))
		m_dac[0]->write(m_video_ram[(m_last_scanline) * 256 + 160]);

	if (!(m_dac_control & 0x02))
		m_dac[1]->write(m_video_ram[(m_last_scanline) * 256 + 161]);

	m_last_scanline = scanline;

	scanline = (scanline+1) % 256;

	/* come back at the next appropriate scanline */
	m_scanline_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


/*************************************
 *
 *  ROM-based tilemap
 *
 *************************************/

TILEMAP_MAPPER_MEMBER(leland_state::leland_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0xff) | ((row & 0x1f) << 8) | ((row & 0xe0) << 9);
}

TILE_GET_INFO_MEMBER(leland_state::leland_get_tile_info)
{
	int char_bank = ((m_gfxbank >> 4) & 0x03) << 10;
	int prom_bank = ((m_gfxbank >> 3) & 0x01) << 13;
	int tile = m_bg_prom[prom_bank | tile_index] | ((tile_index >> 7) & 0x300) | char_bank;
	SET_TILE_INFO_MEMBER(0, tile, m_bg_prom[prom_bank | tile_index] >> 5, 0);
}


/*************************************
 *
 *  RAM-based tilemap
 *
 *************************************/

TILEMAP_MAPPER_MEMBER(ataxx_state::ataxx_scan)
{
	/* logical (col,row) -> memory offset */
	return (col & 0xff) | ((row & 0x3f) << 8) | ((row & 0x40) << 9);
}

TILE_GET_INFO_MEMBER(ataxx_state::ataxx_get_tile_info)
{
	uint16_t tile = m_ataxx_qram[tile_index] | ((m_ataxx_qram[0x4000 | tile_index] & 0x7f) << 8);
	SET_TILE_INFO_MEMBER(0, tile, 0, 0);
}


/*************************************
 *
 *  Start video hardware
 *
 *************************************/

void leland_state::video_start()
{
	/* tilemap */
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(leland_state::leland_get_tile_info),this), tilemap_mapper_delegate(FUNC(leland_state::leland_scan),this), 8, 8, 256, 256);

	/* allocate memory */
	m_video_ram = make_unique_clear<uint8_t[]>(VRAM_SIZE);

	/* scanline timer */
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(leland_state::scanline_callback),this));
	m_scanline_timer->adjust(m_screen->time_until_pos(0));

	save_item(NAME(m_gfx_control));
	save_pointer(NAME(m_video_ram), VRAM_SIZE);
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_last_scanline));
	for (uint8_t i = 0; i < 2; i++)
	{
		save_item(NAME(m_vram_state[i].m_addr), i);
		save_item(NAME(m_vram_state[i].m_latch), i);
	}
}

void ataxx_state::video_start()
{
	// TODO: further untangle driver so the base class doesn't have stuff that isn't common and this can call the base implementation
	/* tilemap */
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ataxx_state::ataxx_get_tile_info),this), tilemap_mapper_delegate(FUNC(ataxx_state::ataxx_scan),this), 8, 8, 256, 128);

	/* first do the standard stuff */
	m_video_ram = make_unique_clear<uint8_t[]>(VRAM_SIZE);

	/* allocate memory */
	m_ataxx_qram = make_unique_clear<uint8_t[]>(QRAM_SIZE);

	save_pointer(NAME(m_video_ram), VRAM_SIZE);
	save_pointer(NAME(m_ataxx_qram), QRAM_SIZE);
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	for (uint8_t i = 0; i < 2; i++)
	{
		save_item(NAME(m_vram_state[i].m_addr), i);
		save_item(NAME(m_vram_state[i].m_latch), i);
	}
}


/*************************************
 *
 *  Scrolling and banking
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_scroll_w)
{
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	/* adjust the proper scroll value */
	switch (offset)
	{
		case 0:
			m_xscroll = (m_xscroll & 0xff00) | (data & 0x00ff);
			break;

		case 1:
			m_xscroll = (m_xscroll & 0x00ff) | ((data << 8) & 0xff00);
			break;

		case 2:
			m_yscroll = (m_yscroll & 0xff00) | (data & 0x00ff);
			break;

		case 3:
			m_yscroll = (m_yscroll & 0x00ff) | ((data << 8) & 0xff00);
			break;

		default:
			fatalerror("Unexpected leland_gfx_port_w\n");
	}
}


WRITE8_MEMBER(leland_state::leland_gfx_port_w)
{
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	if (m_gfxbank != data)
	{
		m_gfxbank = data;
		m_tilemap->mark_all_dirty();
	}
}


/*************************************
 *
 *  Video address setting
 *
 *************************************/

void leland_state::leland_video_addr_w(address_space &space, int offset, int data, int num)
{
	struct vram_state_data &state = m_vram_state[num];

	if (!offset)
		state.m_addr = (state.m_addr & 0xfe00) | ((data << 1) & 0x01fe);
	else
		state.m_addr = ((data << 9) & 0xfe00) | (state.m_addr & 0x01fe);
}


/*************************************
 *
 *  Common video RAM read
 *
 *************************************/

int leland_state::leland_vram_port_r(address_space &space, int offset, int num)
{
	struct vram_state_data *state = m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int ret;

	switch (offset & 7)
	{
		case 3: /* read hi/lo (alternating) */
			ret = m_video_ram[addr];
			addr += inc & (addr << 1);
			addr ^= 1;
			break;

		case 5: /* read hi */
			ret = m_video_ram[addr | 1];
			addr += inc;
			break;

		case 6: /* read lo */
			ret = m_video_ram[addr & ~1];
			addr += inc;
			break;

		default:
			logerror("%s: Warning: Unknown video port %02x read (address=%04x)\n",
						machine().describe_context(), offset, addr);
			ret = 0;
			break;
	}
	state->m_addr = addr;

	if (LOG_COMM && addr >= 0xf000)
		logerror("%s:%s comm read %04X = %02X\n", machine().describe_context(), num ? "slave" : "master", addr, ret);

	return ret;
}


/*************************************
 *
 *  Common video RAM write
 *
 *************************************/

void leland_state::leland_vram_port_w(address_space &space, int offset, int data, int num)
{
	uint8_t *video_ram = m_video_ram.get();
	struct vram_state_data *state = m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int trans = (offset >> 4) & num;

	/* don't fully understand why this is needed.  Isn't the
	   video RAM just one big RAM? */
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);

	if (LOG_COMM && addr >= 0xf000)
		logerror("%s:%s comm write %04X = %02X\n", machine().describe_context(), num ? "slave" : "master", addr, data);

	/* based on the low 3 bits of the offset, update the destination */
	switch (offset & 7)
	{
		case 1: /* write hi = data, lo = latch */
			video_ram[addr & ~1] = state->m_latch[0];
			video_ram[addr |  1] = data;
			addr += inc;
			break;

		case 2: /* write hi = latch, lo = data */
			video_ram[addr & ~1] = data;
			video_ram[addr |  1] = state->m_latch[1];
			addr += inc;
			break;

		case 3: /* write hi/lo = data (alternating) */
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr] & 0x0f;
			}
			video_ram[addr] = data;
			addr += inc & (addr << 1);
			addr ^= 1;
			break;

		case 5: /* write hi = data */
			state->m_latch[1] = data;
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr | 1] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr | 1] & 0x0f;
			}
			video_ram[addr | 1] = data;
			addr += inc;
			break;

		case 6: /* write lo = data */
			state->m_latch[0] = data;
			if (trans)
			{
				if (!(data & 0xf0)) data |= video_ram[addr & ~1] & 0xf0;
				if (!(data & 0x0f)) data |= video_ram[addr & ~1] & 0x0f;
			}
			video_ram[addr & ~1] = data;
			addr += inc;
			break;

		default:
			logerror("%s:Warning: Unknown video port write (address=%04x value=%02x)\n",
						machine().describe_context(), offset, addr);
			break;
	}

	/* update the address and plane */
	state->m_addr = addr;
}


/*************************************
 *
 *  Master video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_master_video_addr_w)
{
	leland_video_addr_w(space, offset, data, 0);
}


TIMER_CALLBACK_MEMBER(leland_state::leland_delayed_mvram_w)
{
	address_space &space = m_master->space(AS_PROGRAM);

	int num = (param >> 16) & 1;
	int offset = (param >> 8) & 0xff;
	int data = param & 0xff;
	leland_vram_port_w(space, offset, data, num);
}


WRITE8_MEMBER(leland_state::leland_mvram_port_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(leland_state::leland_delayed_mvram_w),this), 0x00000 | (offset << 8) | data);
}


READ8_MEMBER(leland_state::leland_mvram_port_r)
{
	return leland_vram_port_r(space, offset, 0);
}


/*************************************
 *
 *  Slave video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(leland_state::leland_slave_video_addr_w)
{
	leland_video_addr_w(space, offset, data, 1);
}


WRITE8_MEMBER(leland_state::leland_svram_port_w)
{
	leland_vram_port_w(space, offset, data, 1);
}


READ8_MEMBER(leland_state::leland_svram_port_r)
{
	return leland_vram_port_r(space, offset, 1);
}


/*************************************
 *
 *  Ataxx master video RAM read/write
 *
 *************************************/

WRITE8_MEMBER(ataxx_state::ataxx_mvram_port_w)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(ataxx_state::leland_delayed_mvram_w),this), 0x00000 | (offset << 8) | data);
}


WRITE8_MEMBER(ataxx_state::ataxx_svram_port_w)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	leland_vram_port_w(space, offset, data, 1);
}


/*************************************
 *
 *  Ataxx slave video RAM read/write
 *
 *************************************/

READ8_MEMBER(ataxx_state::ataxx_mvram_port_r)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return leland_vram_port_r(space, offset, 0);
}


READ8_MEMBER(ataxx_state::ataxx_svram_port_r)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return leland_vram_port_r(space, offset, 1);
}


/*************************************
 *
 *  Refresh routine
 *
 *************************************/

uint32_t leland_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrollx(0, m_xscroll);
	m_tilemap->set_scrolly(0, m_yscroll);
	m_tilemap->draw(screen, bitmap, cliprect, 0);

	/* for each scanline in the visible region */
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint16_t *const dst = &bitmap.pix16(y);
		uint8_t const *const fg_src = &m_video_ram[y << 8];

		/* for each pixel on the scanline */
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			/* build the pen, background is d0-d5 */
			pen_t pen = dst[x] & 0x3f;

			/* foreground is d6-d9 */
			if (x & 0x01)
				pen = pen | ((fg_src[x >> 1] & 0x0f) << 6);
			else
				pen = pen | ((fg_src[x >> 1] & 0xf0) << 2);

			dst[x] = pen;
		}
	}

	return 0;
}


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static const gfx_layout leland_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout ataxx_layout =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(5,6), RGN_FRAC(4,6), RGN_FRAC(3,6), RGN_FRAC(2,6), RGN_FRAC(1,6), RGN_FRAC(0,6) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_leland )
	GFXDECODE_ENTRY( "bg_gfx", 0, leland_layout, 0, 8*16) // *16 is foreground
GFXDECODE_END

static GFXDECODE_START( gfx_ataxx )
	GFXDECODE_ENTRY( "bg_gfx", 0, ataxx_layout, 0, 16) // 16 is foreground
GFXDECODE_END

void leland_state::leland_video(machine_config &config)
{
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_leland);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233, 1024);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(leland_state::screen_update));
	m_screen->set_palette(m_palette);
}

void ataxx_state::ataxx_video(machine_config &config)
{
	leland_video(config);

	GFXDECODE(config.replace(), m_gfxdecode, m_palette, gfx_ataxx);
	m_palette->set_format(palette_device::xRGB_444, 1024);
}
