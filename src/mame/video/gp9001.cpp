// license:BSD-3-Clause
// copyright-holders:Quench, David Haywood
/* GP9001 Video Controller */

/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use one or more Toaplan L7A0498 GP9001 graphic controllers.

  The simpler hardware of these games use one GP9001 controller.

  Next we have games that use two GP9001 controllers, the mixing of
  the VDPs depends on a PAL on the motherboard.
  (mixing handled in toaplan2.cpp)

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
    -  Measure cycle usage / max usable cycle for sprite drawing




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
0003, 0002, 4000 | 0003, 0002, 4000 | 0202, 0203, 4200 | 0003, 0002, 4000 |

Dogyuun          | Batsugun         |
0202, 0203, 4200 | 0202, 0203, 4200 |
1202, 1203, 5200 | 1202, 1203, 5200 | <--- Second video controller

Pipi & Bibis     | Fix Eight        | V-Five           | Snow Bros. 2     |
0003, 0002, 4000 | 0202, 0203, 4200 | 0202, 0203, 4200 | 0202, 0203, 4200 |

Enma Daio        | Power Kick       | Othello Derby    | Sorcer Striker   |
0202, 0203, 4200 | 0003, 0002, 4000 | 0003, 0002, 4000 | 0003, 0002, 4000 |

Kingdom GrandP.  | Battle Garegga   | Batrider         | Battle Bakraid   |
0003, 0002, 4000 | 0003, 0002, 4000 | 0003, 0002, 4000 | 0003, 0002, 4000 |

***************************************************************************/


#include "emu.h"
#include "gp9001.h"
#include "screen.h"

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

 Batrider Gob-Robo boss however definitely requires SPRITES to still have 16 levels of priority against other
 sprites, see http://mametesters.org/view.php?id=5832

 It is unknown if the current solution breaks anything.  The majority of titles don't make extensive use of the
 priority system.

*/
static constexpr unsigned GP9001_PRIMASK = 0x000f;
static constexpr unsigned GP9001_PRIMASK_TMAPS = 0x000e;

template<int Layer>
void gp9001vdp_device::tmap_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tm[Layer].tmap->mark_tile_dirty(offset >> 1);
}


void gp9001vdp_device::map(address_map &map)
{
	map(0x0000, 0x0fff).ram().w(FUNC(gp9001vdp_device::tmap_w<0>)).share("vram_0");
	map(0x1000, 0x1fff).ram().w(FUNC(gp9001vdp_device::tmap_w<1>)).share("vram_1");
	map(0x2000, 0x2fff).ram().w(FUNC(gp9001vdp_device::tmap_w<2>)).share("vram_2");
	map(0x3000, 0x37ff).ram().share("spriteram").mirror(0x0800);
//  map(0x3800, 0x3fff).ram(); // sprite mirror?
}

// each 16x16 tile is actually 4 8x8 tile groups.
static const gfx_layout layout_16x16 =
{
	16,16,          /* 16x16 */
	RGN_FRAC(1,2),  /* Number of tiles */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1), STEP8(8*8*2,1) },
	{ STEP8(0,8*2), STEP8(16*8*2,8*2) },
	16*16*2
};

static const gfx_layout layout =
{
	8,8,            /* 8x8 */
	RGN_FRAC(1,2),  /* Number of 8x8 sprites */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

GFXDECODE_MEMBER( gp9001vdp_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout_16x16, 0, 0x1000 )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout,       0, 0x1000 )
GFXDECODE_END


DEFINE_DEVICE_TYPE(GP9001_VDP, gp9001vdp_device, "gp9001vdp", "GP9001 VDP")

gp9001vdp_device::gp9001vdp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GP9001_VDP, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo)
	, device_video_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("gp9001vdp", ENDIANNESS_BIG, 16, 14, 0, address_map_constructor(FUNC(gp9001vdp_device::map), this))
	, m_vram(*this, "vram_%u", 0)
	, m_spriteram(*this, "spriteram")
	, m_gp9001_cb(*this)
	, m_vint_out_cb(*this)
{
}

device_memory_interface::space_config_vector gp9001vdp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

template<int Layer>
TILE_GET_INFO_MEMBER(gp9001vdp_device::get_tile_info)
{
	const u32 attrib = m_vram[Layer][(tile_index << 1)];

	u32 tile_number = m_vram[Layer][(tile_index << 1) | 1];
	if (!m_gp9001_cb.isnull())
	{
		// each tile is 4 8x8 tile group, actually tile number for each tile is << 2 of real address
		tile_number <<= 2;
		m_gp9001_cb(Layer, tile_number);
		tile_number >>= 2;
	}

	const u32 color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	tileinfo.set(0,
			tile_number,
			color,
			0);
	//tileinfo.category = (attrib & 0x0f00) >> 8;
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void gp9001vdp_device::device_add_mconfig(machine_config &config)
{
	BUFFERED_SPRITERAM16(config, m_spriteram);
}

void gp9001vdp_device::create_tilemaps()
{
	m_tm[2].tmap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(gp9001vdp_device::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16,16,32,32);
	m_tm[1].tmap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(gp9001vdp_device::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16,32,32);
	m_tm[0].tmap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(gp9001vdp_device::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16,32,32);

	m_tm[2].tmap->set_transparent_pen(0);
	m_tm[1].tmap->set_transparent_pen(0);
	m_tm[0].tmap->set_transparent_pen(0);
}


void gp9001vdp_device::device_start()
{
	create_tilemaps();

	m_gp9001_cb.resolve();
	m_vint_out_cb.resolve();

	m_raise_irq_timer = timer_alloc(TIMER_RAISE_IRQ);

	save_item(NAME(m_scroll_reg));
	save_item(NAME(m_voffs));
	for (int i = 0; i < 3; i++)
	{
		save_item(NAME(m_tm[i].scrollx), i);
		save_item(NAME(m_tm[i].scrolly), i);
		save_item(NAME(m_tm[i].flip), i);
	}
	save_item(NAME(m_sp.scrollx));
	save_item(NAME(m_sp.scrolly));
	save_item(NAME(m_sp.flip));

	m_gfxrom_bank_dirty = false;

	// default layer offsets used by all original games
	m_tm[0].extra_xoffset.normal  = -0x1d6;
	m_tm[0].extra_xoffset.flipped = -0x229;
	m_tm[0].extra_yoffset.normal  = -0x1ef;
	m_tm[0].extra_yoffset.flipped = -0x210;

	m_tm[1].extra_xoffset.normal  = -0x1d8;
	m_tm[1].extra_xoffset.flipped = -0x227;
	m_tm[1].extra_yoffset.normal  = -0x1ef;
	m_tm[1].extra_yoffset.flipped = -0x210;

	m_tm[2].extra_xoffset.normal = -0x1da;
	m_tm[2].extra_xoffset.flipped= -0x225;
	m_tm[2].extra_yoffset.normal = -0x1ef;
	m_tm[2].extra_yoffset.flipped= -0x210;

	m_sp.extra_xoffset.normal  = -0x1cc;
	m_sp.extra_xoffset.flipped = -0x17b;
	m_sp.extra_yoffset.normal  = -0x1ef;
	m_sp.extra_yoffset.flipped = -0x108;

	m_sp.use_sprite_buffer = 1;
}

void gp9001vdp_device::device_reset()
{
	m_voffs = 0;
	m_scroll_reg = 0;
	m_tm[0].scrollx = m_tm[0].scrolly = 0;
	m_tm[1].scrollx = m_tm[1].scrolly = 0;
	m_tm[2].scrollx = m_tm[2].scrolly = 0;
	m_sp.scrollx = m_sp.scrolly = 0;

	m_tm[0].flip = 0;
	m_tm[1].flip = 0;
	m_tm[2].flip = 0;
	m_sp.flip = 0;

	init_scroll_regs();

	if (!m_vint_out_cb.isnull())
		m_vint_out_cb(0);
	m_raise_irq_timer->adjust(attotime::never);
}


void gp9001vdp_device::voffs_w(u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_voffs);
}

int gp9001vdp_device::videoram16_r()
{
	const int offs = m_voffs;
	if (!machine().side_effects_disabled())
		m_voffs++;

	return space().read_word(offs*2);
}


void gp9001vdp_device::videoram16_w(u16 data, u16 mem_mask)
{
	const int offs = m_voffs;
	if (!machine().side_effects_disabled())
		m_voffs++;

	space().write_word(offs*2, data, mem_mask);
}


u16 gp9001vdp_device::vdpstatus_r()
{
	return ((screen().vpos() + 15) % 262) >= 245;
}

void gp9001vdp_device::scroll_reg_select_w(u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_scroll_reg = data & 0x8f;
		if (data & 0x70)
			logerror("Hmmm, selecting unknown LSB video control register (%04x)\n",m_scroll_reg);
	}
	else
	{
		logerror("Hmmm, selecting unknown MSB video control register (%04x)\n",m_scroll_reg);
	}
}

void gp9001vdp_device::tilemaplayer::set_scrollx_and_flip_reg(u16 data, u16 mem_mask, bool f)
{
	COMBINE_DATA(&scrollx);

	if (f)
	{
		flip |= TILEMAP_FLIPX;
		tmap->set_scrollx(0, -(scrollx + extra_xoffset.flipped));
	}
	else
	{
		flip &= ~TILEMAP_FLIPX;
		tmap->set_scrollx(0, scrollx + extra_xoffset.normal);
	}
	tmap->set_flip(flip);
}

void gp9001vdp_device::tilemaplayer::set_scrolly_and_flip_reg(u16 data, u16 mem_mask, bool f)
{
	COMBINE_DATA(&scrolly);

	if (f)
	{
		flip |= TILEMAP_FLIPY;
		tmap->set_scrolly(0, -(scrolly + extra_yoffset.flipped));

	}
	else
	{
		flip &= ~TILEMAP_FLIPY;
		tmap->set_scrolly(0, scrolly + extra_yoffset.normal);
	}

	tmap->set_flip(flip);
}

void gp9001vdp_device::spritelayer::set_scrollx_and_flip_reg(u16 data, u16 mem_mask, bool f)
{
	if (f)
	{
		data += extra_xoffset.flipped;
		COMBINE_DATA(&scrollx);
		if (scrollx & 0x8000) scrollx |= 0xfe00;
		else scrollx &= 0x1ff;
		flip |= SPRITE_FLIPX;
	}
	else
	{
		data += extra_xoffset.normal;
		COMBINE_DATA(&scrollx);

		if (scrollx & 0x8000) scrollx |= 0xfe00;
		else scrollx &= 0x1ff;
		flip &= ~SPRITE_FLIPX;
	}
}

void gp9001vdp_device::spritelayer::set_scrolly_and_flip_reg(u16 data, u16 mem_mask, bool f)
{
	if (f)
	{
		data += extra_yoffset.flipped;
		COMBINE_DATA(&scrolly);
		if (scrolly & 0x8000) scrolly |= 0xfe00;
		else scrolly &= 0x1ff;
		flip |= SPRITE_FLIPY;
	}
	else
	{
		data += extra_yoffset.normal;
		COMBINE_DATA(&scrolly);
		if (scrolly & 0x8000) scrolly |= 0xfe00;
		else scrolly &= 0x1ff;
		flip &= ~SPRITE_FLIPY;
	}
}

void gp9001vdp_device::scroll_reg_data_w(u16 data, u16 mem_mask)
{
	/************************************************************************/
	/***** layer X and Y flips can be set independently, so emulate it ******/
	/************************************************************************/

	// writes with 8x set turn on flip for the specified layer / axis
	int flip = m_scroll_reg & 0x80;

	switch (m_scroll_reg&0x7f)
	{
		case 0x00: m_tm[0].set_scrollx_and_flip_reg(data, mem_mask, flip); break;
		case 0x01: m_tm[0].set_scrolly_and_flip_reg(data, mem_mask, flip); break;

		case 0x02: m_tm[1].set_scrollx_and_flip_reg(data, mem_mask, flip); break;
		case 0x03: m_tm[1].set_scrolly_and_flip_reg(data, mem_mask, flip); break;

		case 0x04: m_tm[2].set_scrollx_and_flip_reg(data, mem_mask, flip); break;
		case 0x05: m_tm[2].set_scrolly_and_flip_reg(data, mem_mask, flip); break;

		case 0x06: m_sp.set_scrollx_and_flip_reg(data, mem_mask, flip); break;
		case 0x07: m_sp.set_scrolly_and_flip_reg(data, mem_mask, flip); break;


		case 0x0e:  /******* Initialise video controller register ? *******/

		case 0x0f: if (!m_vint_out_cb.isnull()) m_vint_out_cb(0); break;


		default:    logerror("Hmmm, writing %08x to unknown video control register (%08x) !!!\n",data,m_scroll_reg);
					break;
	}
}

void gp9001vdp_device::init_scroll_regs()
{
	m_tm[0].set_scrollx_and_flip_reg(0, 0xffff, 0);
	m_tm[0].set_scrolly_and_flip_reg(0, 0xffff, 0);
	m_tm[1].set_scrollx_and_flip_reg(0, 0xffff, 0);
	m_tm[1].set_scrolly_and_flip_reg(0, 0xffff, 0);
	m_tm[2].set_scrollx_and_flip_reg(0, 0xffff, 0);
	m_tm[2].set_scrolly_and_flip_reg(0, 0xffff, 0);
	m_sp.set_scrollx_and_flip_reg(0, 0xffff, 0);
	m_sp.set_scrolly_and_flip_reg(0, 0xffff, 0);
}



u16 gp9001vdp_device::read(offs_t offset, u16 mem_mask)
{
	switch (offset & (0xc/2))
	{
		case 0x04/2:
			return videoram16_r();

		case 0x0c/2:
			return vdpstatus_r();

		default:
			logerror("read: read from unhandled offset %04x\n",offset*2);
	}

	return 0xffff;
}

void gp9001vdp_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset & (0xc/2))
	{
		case 0x00/2:
			voffs_w(data, mem_mask);
			break;

		case 0x04/2:
			videoram16_w(data, mem_mask);
			break;

		case 0x08/2:
			scroll_reg_select_w(data, mem_mask);
			break;

		case 0x0c/2:
			scroll_reg_data_w(data, mem_mask);
			break;
	}
}


/***************************************************************************/
/**************** PIPIBIBI bootleg interface into this video driver ********/

void gp9001vdp_device::pipibibi_bootleg_scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
	{
		switch (offset)
		{
			case 0x00:  data -= 0x01f; break;
			case 0x01:  data += 0x1ef; break;
			case 0x02:  data -= 0x01d; break;
			case 0x03:  data += 0x1ef; break;
			case 0x04:  data -= 0x01b; break;
			case 0x05:  data += 0x1ef; break;
			case 0x06:  data += 0x1d4; break;
			case 0x07:  data += 0x1f7; break;
			default:    logerror("PIPIBIBI writing %04x to unknown scroll register %04x",data, offset);
		}

		m_scroll_reg = offset;
		scroll_reg_data_w(data, mem_mask);
	}
}

u16 gp9001vdp_device::pipibibi_bootleg_videoram16_r(offs_t offset)
{
	voffs_w(offset, 0xffff);
	return videoram16_r();
}

void gp9001vdp_device::pipibibi_bootleg_videoram16_w(offs_t offset, u16 data, u16 mem_mask)
{
	voffs_w(offset, 0xffff);
	videoram16_w(data, mem_mask);
}

u16 gp9001vdp_device::pipibibi_bootleg_spriteram16_r(offs_t offset)
{
	voffs_w((0x1800 + offset), 0);
	return videoram16_r();
}

void gp9001vdp_device::pipibibi_bootleg_spriteram16_w(offs_t offset, u16 data, u16 mem_mask)
{
	voffs_w((0x1800 + offset), mem_mask);
	videoram16_w(data, mem_mask);
}

/***************************************************************************
    Blanking Signal Polling
***************************************************************************/

READ_LINE_MEMBER(gp9001vdp_device::hsync_r)
{
	int hpos = screen().hpos();

	// active low output
	return (hpos > 325) && (hpos < 380) ? 0 : 1;
}

READ_LINE_MEMBER(gp9001vdp_device::vsync_r)
{
	int vpos = screen().vpos();

	// active low output
	return (vpos >= 232) && (vpos <= 245) ? 0 : 1;
}

READ_LINE_MEMBER(gp9001vdp_device::fblank_r)
{
	// ?? Dogyuun is too slow if this is wrong
	return (hsync_r() == 0 || vsync_r() == 0) ? 0 : 1;
}

/***************************************************************************
    Sprite Handlers
***************************************************************************/

void gp9001vdp_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, const u8* primap )
{
	int clk = 0;
	int clk_max = 432 * 262; // TODO : related to size of whole screen?
	const u16 *source = (m_sp.use_sprite_buffer) ? m_spriteram->buffer() : m_spriteram->live();

	const u32 total_elements = gfx(1)->elements();
	const u32 total_colors = gfx(1)->colors();

	int old_x = (-(m_sp.scrollx)) & 0x1ff;
	int old_y = (-(m_sp.scrolly)) & 0x1ff;

	for (int offs = 0; offs < (m_spriteram->bytes() / 2); offs += 4)
	{
		clk += 8; // 8 cycle per each sprite
		if (clk > clk_max)
			return;

		int sx, sy;
		int sx_base, sy_base;

		const u16 attrib = source[offs];
		const int priority = primap[(attrib >> 8) & GP9001_PRIMASK] + 1;

		if ((attrib & 0x8000))
		{
			u32 sprite = ((attrib & 3) << 16) | source[offs + 1];   /* 18 bit */
			if (!m_gp9001_cb.isnull())        /* Batrider Sprite select bank switching required */
				m_gp9001_cb(3, sprite);

			u32 color = (attrib >> 2) & 0x3f;

			/***** find out sprite size *****/
			const int sprite_sizex = ((source[offs + 2] & 0x0f) + 1) * 8;
			const int sprite_sizey = ((source[offs + 3] & 0x0f) + 1) * 8;

			/***** find position to display sprite *****/
			if (!(attrib & 0x4000))
			{
				sx_base = ((source[offs + 2] >> 7) - (m_sp.scrollx)) & 0x1ff;
				sy_base = ((source[offs + 3] >> 7) - (m_sp.scrolly)) & 0x1ff;
			}
			else
			{
				sx_base = (old_x + (source[offs + 2] >> 7)) & 0x1ff;
				sy_base = (old_y + (source[offs + 3] >> 7)) & 0x1ff;
			}

			old_x = sx_base;
			old_y = sy_base;

			int flipx = attrib & SPRITE_FLIPX;
			int flipy = attrib & SPRITE_FLIPY;

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
			if (m_sp.flip)
			{
				if (m_sp.flip & SPRITE_FLIPX)
					sx_base = 320 - sx_base;
				if (m_sp.flip & SPRITE_FLIPY)
					sy_base = 240 - sy_base;
			}

			/***** Cancel flip, if it, and sprite layer flip are active *****/
			flipx = (flipx ^ (m_sp.flip & SPRITE_FLIPX));
			flipy = (flipy ^ (m_sp.flip & SPRITE_FLIPY));

			/***** Draw the complete sprites using the dimension info *****/
			for (int dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				if (flipy) sy = sy_base - dim_y;
				else       sy = sy_base + dim_y;
				for (int dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					clk += 32; // 32? cycle per each tile; TODO: verify from real hardware
					if (clk > clk_max)
						return;

					if (flipx) sx = sx_base - dim_x;
					else       sx = sx_base + dim_x;

					/*
					gfx->transpen_raw(bitmap,cliprect,sprite,
					    color << 4,
					    flipx,flipy,
					    sx,sy,0);
					*/
					sprite %= total_elements;
					color %= total_colors;
					const pen_t *paldata = &palette().pen(color * 16);
					{
						const u8* srcdata = gfx(1)->get_data(sprite);
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

						for (int yy = ystart; yy != yend; yy += yinc)
						{
							const int drawyy = yy + sy;

							for (int xx = xstart; xx != xend; xx += xinc)
							{
								const int drawxx = xx + sx;

								if (cliprect.contains(drawxx, drawyy))
								{
									const u8 pix = srcdata[count];
									u16 *const dstptr = &bitmap.pix(drawyy, drawxx);
									u8 *const dstpri = &this->custom_priority_bitmap->pix(drawyy, drawxx);

									if (priority >= dstpri[0])
									{
										if (pix & 0xf)
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

					sprite++;
				}
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_ind16.
***************************************************************************/

void gp9001vdp_device::draw_custom_tilemap( bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, const u8* priremap, const u8* pri_enable )
{
	tilemap_t* tilemap = m_tm[layer].tmap;
	bitmap_ind16 &tmb = tilemap->pixmap();

	const int scrollx = tilemap->scrollx(0);
	const int scrolly = tilemap->scrolly(0);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const int realy = (y + scrolly) & 0x1ff;

		u16 const *const srcptr = &tmb.pix(realy);
		u16 *const dstptr = &bitmap.pix(y);
		u8 *const dstpriptr = &this->custom_priority_bitmap->pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const int realx = (x + scrollx) & 0x1ff;

			u16 pixdat = srcptr[realx];
			u8 pixpri = ((pixdat >> 12) & GP9001_PRIMASK_TMAPS);

			if (pri_enable[pixpri])
			{
				pixpri = priremap[pixpri] + 1; // priority of 0 isn't desireable
				pixdat &= 0x07ff;

				if (pixdat & 0xf)
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


static const u8 gp9001_primap1[16] =  { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c };
//static const u8 gp9001_sprprimap1[16] =  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static const u8 gp9001_sprprimap1[16] =  { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c };

static const u8 batsugun_prienable0[16]={ 1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1 };

void gp9001vdp_device::render_vdp(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_gfxrom_bank_dirty)
	{
		for (int i = 0; i < 3; i++)
			m_tm[i].tmap->mark_all_dirty();

		m_gfxrom_bank_dirty = false;
	}

	draw_custom_tilemap(bitmap, cliprect, 0, gp9001_primap1, batsugun_prienable0);
	draw_custom_tilemap(bitmap, cliprect, 1, gp9001_primap1, batsugun_prienable0);
	draw_custom_tilemap(bitmap, cliprect, 2, gp9001_primap1, batsugun_prienable0);
	draw_sprites(bitmap, cliprect, gp9001_sprprimap1);
}


void gp9001vdp_device::screen_eof(void)
{
	/** Shift sprite RAM buffers  ***  Used to fix sprite lag **/
	if (m_sp.use_sprite_buffer) m_spriteram->copy();

	// the IRQ appears to fire at line 0xe6
	if (!m_vint_out_cb.isnull())
		m_raise_irq_timer->adjust(screen().time_until_pos(0xe6));
}


void gp9001vdp_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RAISE_IRQ:
		m_vint_out_cb(1);
		break;
	default:
		throw emu_fatalerror("Unknown id in gp9001vdp_device::device_timer");
	}
}
