// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cinematronics / Leland Cinemat System driver

    Leland video hardware

***************************************************************************/

#include "emu.h"
#include "leland.h"
#include "leland_a.h"

/* debugging */
#define LOG_WARN    (1U << 1)
#define LOG_COMM    (1U << 2)

#define VERBOSE     LOG_WARN
#include "logmacro.h"


/* constants */
static constexpr int VRAM_SIZE = 0x10000;
static constexpr int QRAM_SIZE = 0x10000;


/*************************************
 *
 *  Scanline callback
 *
 *************************************/

TIMER_CALLBACK_MEMBER(leland_state::scanline_callback)
{
	u8 scanline = param;
	u8 last_scanline = scanline - 1;

	/* update the DACs */
	if (!(m_dac_control & 0x01))
		m_dac[0]->write(m_video_ram[last_scanline << 8 | 0xa0]);

	if (!(m_dac_control & 0x02))
		m_dac[1]->write(m_video_ram[last_scanline << 8 | 0xa1]);

	scanline++;

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
	tileinfo.set(0, tile, m_bg_prom[prom_bank | tile_index] >> 5, 0);
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
	u16 tile = m_ataxx_qram[tile_index] | ((m_ataxx_qram[0x4000 | tile_index] & 0x7f) << 8);
	tileinfo.set(0, tile, 0, 0);
}


/*************************************
 *
 *  Start video hardware
 *
 *************************************/

void leland_state::video_start()
{
	/* tilemap */
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(leland_state::leland_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(leland_state::leland_scan)), 8, 8, 256, 256);

	/* allocate memory */
	m_video_ram = make_unique_clear<u8[]>(VRAM_SIZE);

	/* scanline timer */
	m_scanline_timer = timer_alloc(FUNC(leland_state::scanline_callback), this);
	m_scanline_timer->adjust(m_screen->time_until_pos(0));

	save_item(NAME(m_gfx_control));
	save_pointer(NAME(m_video_ram), VRAM_SIZE);
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	save_item(NAME(m_gfxbank));
	for (u8 i = 0; i < 2; i++)
	{
		save_item(NAME(m_vram_state[i].m_addr), i);
		save_item(NAME(m_vram_state[i].m_latch), i);
	}
}

void ataxx_state::video_start()
{
	// TODO: further untangle driver so the base class doesn't have stuff that isn't common and this can call the base implementation
	/* tilemap */
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ataxx_state::ataxx_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(ataxx_state::ataxx_scan)), 8, 8, 256, 128);

	/* first do the standard stuff */
	m_video_ram = make_unique_clear<u8[]>(VRAM_SIZE);

	/* allocate memory */
	m_ataxx_qram = make_unique_clear<u8[]>(QRAM_SIZE);

	save_pointer(NAME(m_video_ram), VRAM_SIZE);
	save_pointer(NAME(m_ataxx_qram), QRAM_SIZE);
	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
	for (u8 i = 0; i < 2; i++)
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

void leland_state::scroll_w(offs_t offset, u8 data)
{
	m_screen->update_partial(m_screen->vpos() - 1);

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
			fatalerror("Unexpected scroll_w\n");
	}
}


void leland_state::gfx_port_w(u8 data)
{
	if (m_gfxbank != data)
	{
		m_screen->update_partial(m_screen->vpos() - 1);

		m_gfxbank = data;
		m_tilemap->mark_all_dirty();
	}
}


/*************************************
 *
 *  Video address setting
 *
 *************************************/

void leland_state::video_addr_w(offs_t offset, u8 data, int num)
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

int leland_state::vram_port_r(offs_t offset, int num)
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
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_WARN, "%s: Warning: Unknown video port %02x read (address=%04x)\n",
							machine().describe_context(), offset, addr);
			ret = 0;
			break;
	}

	if (!machine().side_effects_disabled())
	{
		state->m_addr = addr;

		if (addr >= 0xf000)
			LOGMASKED(LOG_COMM, "%s:%s comm read %04X = %02X\n", machine().describe_context(), num ? "slave" : "master", addr, ret);
	}

	return ret;
}


/*************************************
 *
 *  Common video RAM write
 *
 *************************************/

void leland_state::vram_port_w(offs_t offset, u8 data, int num)
{
	u8 *video_ram = m_video_ram.get();
	struct vram_state_data *state = m_vram_state + num;
	int addr = state->m_addr;
	int inc = (offset >> 2) & 2;
	int trans = (offset >> 4) & num;

	m_screen->update_partial(m_screen->vpos() - 1);

	if (addr >= 0xf000)
		LOGMASKED(LOG_COMM, "%s:%s comm write %04X = %02X\n", machine().describe_context(), num ? "slave" : "master", addr, data);

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
			LOGMASKED(LOG_WARN, "%s:Warning: Unknown video port write (address=%04x value=%02x)\n",
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

void leland_state::master_video_addr_w(offs_t offset, u8 data)
{
	video_addr_w(offset, data, 0);
}


TIMER_CALLBACK_MEMBER(leland_state::leland_delayed_mvram_w)
{
	int num = (param >> 16) & 1;
	int offset = (param >> 8) & 0xff;
	int data = param & 0xff;
	vram_port_w(offset, data, num);
}


void leland_state::leland_mvram_port_w(offs_t offset, u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(leland_state::leland_delayed_mvram_w),this), 0x00000 | (offset << 8) | data);
}


u8 leland_state::leland_mvram_port_r(offs_t offset)
{
	return vram_port_r(offset, 0);
}


/*************************************
 *
 *  Slave video RAM read/write
 *
 *************************************/

void leland_state::slave_video_addr_w(offs_t offset, u8 data)
{
	video_addr_w(offset, data, 1);
}


void leland_state::leland_svram_port_w(offs_t offset, u8 data)
{
	vram_port_w(offset, data, 1);
}


u8 leland_state::leland_svram_port_r(offs_t offset)
{
	return vram_port_r(offset, 1);
}


/*************************************
 *
 *  Ataxx master video RAM read/write
 *
 *************************************/

void ataxx_state::ataxx_mvram_port_w(offs_t offset, u8 data)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(ataxx_state::leland_delayed_mvram_w),this), 0x00000 | (offset << 8) | data);
}


void ataxx_state::ataxx_svram_port_w(offs_t offset, u8 data)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	vram_port_w(offset, data, 1);
}


/*************************************
 *
 *  Ataxx slave video RAM read/write
 *
 *************************************/

u8 ataxx_state::ataxx_mvram_port_r(offs_t offset)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return vram_port_r(offset, 0);
}


u8 ataxx_state::ataxx_svram_port_r(offs_t offset)
{
	offset = ((offset >> 1) & 0x07) | ((offset << 3) & 0x08) | (offset & 0x10);
	return vram_port_r(offset, 1);
}


/*************************************
 *
 *  Refresh routine
 *
 *************************************/

u32 leland_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->set_scrollx(0, m_xscroll);
	m_tilemap->set_scrolly(0, m_yscroll);
	m_tilemap->draw(screen, bitmap, cliprect, 0);

	/* for each scanline in the visible region */
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u16 *const dst = &bitmap.pix(y);
		u8 const *const fg_src = &m_video_ram[y << 8];

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

static GFXDECODE_START( gfx_leland )
	GFXDECODE_ENTRY( "bg_gfx", 0, leland_layout, 0, 8*16) // *16 is foreground
GFXDECODE_END

static GFXDECODE_START( gfx_ataxx )
	GFXDECODE_ENTRY( "bg_gfx", 0, gfx_8x8x6_planar, 0, 16) // 16 is foreground
GFXDECODE_END

void leland_state::leland_video(machine_config &config)
{
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_leland);
	PALETTE(config, m_palette).set_format(palette_device::BGR_233, 1024);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14.318181_MHz_XTAL / 2, 424, 0, 320, 256, 0, 240);
	m_screen->set_screen_update(FUNC(leland_state::screen_update));
	m_screen->set_palette(m_palette);
}

void ataxx_state::ataxx_video(machine_config &config)
{
	leland_video(config);

	GFXDECODE(config.replace(), m_gfxdecode, m_palette, gfx_ataxx);
	m_palette->set_format(palette_device::xRGB_444, 1024);
}
