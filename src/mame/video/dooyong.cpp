// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Vas Crabb
#include "emu.h"
#include "includes/dooyong.h"


/* These games all have ROM-based tilemaps for the backgrounds, title
   screens and sometimes "bosses" and special attacks. There are three
   schemes for tilemap encoding.  The scheme is chosen based on the
   contents of the tilemap control variables declared above.
   Note that although the tilemaps are arbitrarily wide (hundreds of
   thousands of pixels, depending on the size of the ROM), we only
   decode a 1024 pixel wide block at a time, and invalidate the tilemap
   when the x scroll moves out of range (trying to decode the whole lot
   at once uses hundreds of megabytes of RAM). */

device_type const DOOYONG_ROM_TILEMAP = &device_creator<dooyong_rom_tilemap_device>;


dooyong_rom_tilemap_device::dooyong_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DOOYONG_ROM_TILEMAP, "Dooyong ROM Tilemap", tag, owner, clock, "dooyong_rom_tilemap", __FILE__)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_tilerom(*this, finder_base::DUMMY_TAG)
	, m_gfxnum(0)
	, m_tilerom_offset(0)
	, m_transparent_pen(~0U)
	, m_primella_code_mask(0x03ff)
	, m_primella_color_mask(0x3c00)
	, m_primella_color_shift(10)
	, m_tilemap(nullptr)
	, m_registers{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	, m_palette_bank(0)
{
}

void dooyong_rom_tilemap_device::static_set_gfxdecode_tag(device_t &device, char const *tag)
{
	downcast<dooyong_rom_tilemap_device &>(device).m_gfxdecode.set_tag(tag);
}

void dooyong_rom_tilemap_device::static_set_tilerom_tag(device_t &device, char const *tag)
{
	downcast<dooyong_rom_tilemap_device &>(device).m_tilerom.set_tag(tag);
}

void dooyong_rom_tilemap_device::static_set_gfxnum(device_t &device, int gfxnum)
{
	downcast<dooyong_rom_tilemap_device &>(device).m_gfxnum = gfxnum;
}

void dooyong_rom_tilemap_device::static_set_tilerom_offset(device_t &device, int offset)
{
	downcast<dooyong_rom_tilemap_device &>(device).m_tilerom_offset = offset;
}

void dooyong_rom_tilemap_device::static_set_transparent_pen(device_t &device, unsigned pen)
{
	downcast<dooyong_rom_tilemap_device &>(device).m_transparent_pen = pen;
}

void dooyong_rom_tilemap_device::static_set_primella_code_bits(device_t &device, unsigned bits)
{
	dooyong_rom_tilemap_device &tilemap_device(downcast<dooyong_rom_tilemap_device &>(device));
	tilemap_device.m_primella_code_mask = (1U << bits) - 1U;
	tilemap_device.m_primella_color_mask = ((1U << 14) - 1) & ~tilemap_device.m_primella_code_mask;
	tilemap_device.m_primella_color_shift = bits;
}

void dooyong_rom_tilemap_device::draw(screen_device &screen, bitmap_ind16 &dest, rectangle const &cliprect, UINT32 flags, UINT8 priority)
{
	m_tilemap->draw(screen, dest, cliprect, flags, priority);
}

WRITE8_MEMBER(dooyong_rom_tilemap_device::ctrl_w)
{
	UINT8 const old = m_registers[offset];
	if (old != data)
	{
		m_registers[offset] = data;
		switch (offset)
		{
		case 0: // Low byte of x scroll - scroll tilemap
			m_tilemap->set_scrollx(0, data);
			break;
		case 1: // High byte of x scroll - mark tilemap dirty so new tile gfx will be loaded
			m_tilemap->mark_all_dirty();
			break;
		case 3: // Low byte of y scroll
		case 4: // High byte of y scroll
			m_tilemap->set_scrolly(0, m_registers[3] | (unsigned(m_registers[4]) << 8));
			break;
		case 6: // Tilemap enable and mode control
			m_tilemap->enable(!BIT(data, 4));
			if (BIT(data ^ old, 5)) // This sets the tilemap data format
				m_tilemap->mark_all_dirty();
			break;
		default: // Other addresses are used but function is unknown
			// 0x05 and 0x07 are initialised on startup
			// 0x02 is initialised on startup by some games and written to continuously by others
			// printf("Unknown %s tilemap control: 0x%02x = 0x%02x\n", tag(), unsigned(offset), unsigned(data));
			break;
		}
	}
}

void dooyong_rom_tilemap_device::set_palette_bank(UINT16 bank)
{
	if (bank != m_palette_bank)
	{
		m_palette_bank = bank;
		m_tilemap->mark_all_dirty();
	}
}

void dooyong_rom_tilemap_device::device_start()
{
	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(dooyong_rom_tilemap_device::tile_info), this),
			TILEMAP_SCAN_COLS,
			32, 32,
			32, 8);
	if (~m_transparent_pen)
		m_tilemap->set_transparent_pen(m_transparent_pen);

	if (m_tilerom_offset < 0)
		m_tilerom_offset = m_tilerom.length() + m_tilerom_offset;

	std::fill(std::begin(m_registers), std::end(m_registers), 0);
	m_palette_bank = 0U;

	save_item(NAME(m_registers));
	save_item(NAME(m_palette_bank));
}

TILE_GET_INFO_MEMBER(dooyong_rom_tilemap_device::tile_info)
{
	tilemap_memory_index const offs = (tile_index + (unsigned(m_registers[1]) << 6)) << 1;
	unsigned const attr = m_tilerom[m_tilerom_offset + offs];
	unsigned code, color, flags;
	if (BIT(m_registers[6], 5))
	{   // lastday/gulfstrm/pollux/flytiger
		/* Tiles take two bytes in ROM:
		                 MSB   LSB
		   [offs + 0x00] cCCC CYXc    (bit 9 of gfx code, bits 3-0 of color, Y flip, X flip, bit 8 of gfx code)
		   [offs + 0x01] cccc cccc    (bits 7-0 of gfx code)
		   c = gfx code
		   C = color code
		   X = x flip
		   Y = y flip */
		code = m_tilerom[m_tilerom_offset + offs + 1] | (BIT(attr, 0) << 8) | (BIT(attr, 7) << 9);
		color = m_palette_bank | ((attr >> 3) & 0x0fU);
		flags = TILE_FLIPYX((attr >> 1) & 0x03U);
	}
	else
	{   // primella/popbingo
		/* Tiles take two bytes in ROM:
		                 MSB   LSB
		   [offs + 0x00] YXCC CCcc    (Y flip, X flip, bits 3-0 of color code, bits 9-8 of gfx code)
		   [offs + 0x01] cccc cccc    (bits 7-0 of gfx code)
		   c = gfx code
		   C = color code
		   X = x flip
		   Y = y flip */
		code = (attr << 8) | m_tilerom[m_tilerom_offset + offs + 1];
		color = (code & m_primella_color_mask) >> m_primella_color_shift;
		flags = TILE_FLIPYX((attr >> 6) & 0x03U);
		code &= m_primella_code_mask;
	}

	tileinfo.set(m_gfxnum, code, color, flags);
}


inline void dooyong_68k_state::scroll8_w(offs_t offset, UINT8 data, UINT8 *scroll, tilemap_t *map)
{
	UINT8 const old = scroll[offset];
	if (old != data)
	{
		scroll[offset] = data;
		if (map != nullptr) switch (offset)
		{
		case 0: /* Low byte of x scroll - scroll tilemap */
			map->set_scrollx(0, data);
			break;
		case 1: /* High byte of x scroll - mark tilemap dirty so new tile gfx will be loaded */
			map->mark_all_dirty();
			break;
		case 3: /* Low byte of y scroll */
		case 4: /* High byte of y scroll */
			map->set_scrolly(0, (unsigned)scroll[3] | ((unsigned)scroll[4] << 8));
			break;
		case 6: /* Tilemap enable and mode control */
			map->enable(!(data & 0x10));
			if ((data & 0x20) != (old & 0x20))  // This sets the tilemap data format
				map->mark_all_dirty();
			break;
		default:    /* Other addresses are used but function is unknown */
			/* 0x05 and 0x07 are initialised on startup */
			/* 0x02 is initialised on startup by some games and written to continuously by others */
			/*{
			    const char *name;
			    if (scroll == m_bgscroll8)        name = "bg";
			    else if (scroll == m_bg2scroll8)  name = "bg2";
			    else if (scroll == m_fgscroll8)   name = "fg";
			    else if (scroll == m_fg2scroll8)  name = "fg2";
			    else                            name = "unknown";
			    printf("Unknown %s tilemap control: 0x%02x = 0x%02x\n", name, (unsigned)offset, (unsigned)data);
			}*/
			break;
		}
	}
}


/* These handle writes to the tilemap scroll registers.
   There is one per tilemap, wrapping the above function that does the work. */

WRITE8_MEMBER(dooyong_68k_state::bgscroll_w)
{
	scroll8_w(offset, data, m_bgscroll8, m_bg_tilemap);
}

WRITE8_MEMBER(dooyong_68k_state::bg2scroll_w)
{
	scroll8_w(offset, data, m_bg2scroll8, m_bg2_tilemap);
}

WRITE8_MEMBER(dooyong_68k_state::fgscroll_w)
{
	scroll8_w(offset, data, m_fgscroll8, m_fg_tilemap);
}

WRITE8_MEMBER(dooyong_68k_state::fg2scroll_w)
{
	scroll8_w(offset, data, m_fg2scroll8, m_fg2_tilemap);
}


WRITE8_MEMBER(dooyong_z80_state::txvideoram_w)
{
	if (m_txvideoram[offset] != data)
	{
		m_txvideoram[offset] = data;
		if (m_tx_tilemap_mode == 0)
			m_tx_tilemap->mark_tile_dirty(offset & 0x07ff);
		else
			m_tx_tilemap->mark_tile_dirty(offset >> 1);
	}
}


/* Control registers seem to be different on every game */

WRITE8_MEMBER(dooyong_z80_ym2203_state::lastday_ctrl_w)
{
	/* bits 0 and 1 are coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bit 3 is used but unknown */

	/* bit 4 disables sprites */
	m_sprites_disabled = data & 0x10;

	/* bit 6 is flip screen */
	flip_screen_set(data & 0x40);
}

WRITE8_MEMBER(dooyong_z80_ym2203_state::pollux_ctrl_w)
{
//  printf("pollux_ctrl_w %02x\n", data);

	/* bit 0 is flip screen */
	flip_screen_set(data & 0x01);

	/* bits 6 and 7 are coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x80);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);

	/* bit 1 is used but unknown - palette banking (both write and display based on pollux bombs) */
	UINT8 const last_palbank = m_palette_bank;
	if (m_paletteram_flytiger) m_palette_bank = BIT(data, 1);
	if (last_palbank != m_palette_bank)
	{
		m_bg->set_palette_bank(m_palette_bank << 6);
		m_fg->set_palette_bank(m_palette_bank << 6);
		m_tx_tilemap->mark_all_dirty();
	}

	/* bit 2 is continuously toggled (unknown) */

	/* bit 4 is used but unknown - display disable? */
}



WRITE8_MEMBER(dooyong_z80_state::primella_ctrl_w)
{
	/* bits 0-2 select ROM bank */
	membank("bank1")->set_entry(data & 0x07);

	/* bit 3 disables tx layer */
	m_tx_pri = data & 0x08;

	/* bit 4 flips screen */
	flip_screen_set(data & 0x10);

	/* bit 5 used but unknown */

//  logerror("%04x: bankswitch = %02x\n",space.device().safe_pc(),data&0xe0);
}

READ8_MEMBER(dooyong_z80_state::paletteram_flytiger_r)
{
	if (m_palette_bank) offset |= 0x800;

	return m_paletteram_flytiger[offset];
}


WRITE8_MEMBER(dooyong_z80_state::paletteram_flytiger_w)
{
	if (m_palette_bank) offset |= 0x800;

	m_paletteram_flytiger[offset] = data;
	UINT16 const value = m_paletteram_flytiger[offset & ~1] | (m_paletteram_flytiger[offset | 1] << 8);
	m_palette->set_pen_color(offset/2, pal5bit(value >> 10), pal5bit(value >> 5), pal5bit(value >> 0));

}

WRITE8_MEMBER(dooyong_z80_state::flytiger_ctrl_w)
{
	/* bit 0 is flip screen */
	flip_screen_set(data & 0x01);

	/* bits 1, 2 used but unknown */

	/* bit 3 palette banking  */
	UINT8 const last_palbank = m_palette_bank;
	m_palette_bank = BIT(data, 3);
	if (last_palbank != m_palette_bank)
	{
		m_bg->set_palette_bank(m_palette_bank << 6);
		m_fg->set_palette_bank(m_palette_bank << 6);
		m_tx_tilemap->mark_all_dirty();
	}

	/* bit 4 changes tilemaps priority */
	m_flytiger_pri = data & 0x10;
}


TILE_GET_INFO_MEMBER(dooyong_z80_state::get_tx_tile_info)
{
	/* Each tile takes two bytes of memory:
	                 MSB   LSB
	   [offs + 0x00] cccc cccc    (bits 7-0 of gfx code)
	   [offs + 0x01] CCCC cccc    (bits 3-0 of color code, bits 11-8 of gfx code)
	   c = gfx code
	   C = color code */
	unsigned offs, attr;
	if (m_tx_tilemap_mode == 0)
	{   /* lastday/gulfstrm/pollux/flytiger */
		offs = tile_index;
		attr = m_txvideoram[offs | 0x0800];
	}
	else
	{   /* bluehawk/primella */
		offs = tile_index * 2;
		attr = m_txvideoram[offs + 1];
	}
	int const code = m_txvideoram[offs] | ((attr & 0x0f) << 8);
	int const color = (attr & 0xf0) >> 4;

	tileinfo.set(0, code, color | (m_palette_bank << 6), 0);
}


void dooyong_z80_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, unsigned extensions)
{
	/* Sprites take 32 bytes each in memory:
	                 MSB   LSB
	   [offs + 0x00] cccc cccc    (bits 7-0 of gfx code)
	   [offs + 0x01] cccx CCCC    (bits 10-8 of gfx code, bit 8 of x position, bits 3-0 of color code)
	   [offs + 0x02] yyyy yyyy
	   [offs + 0x03] xxxx xxxx    (bits 7-0 of x offset)
	   ...
	   [offs + 0x1c] ?hhh XY*c    (bits 2-0 of height, x flip, y flip, * see note, bit 11 of gfx code)
	   ...
	   ? = unused/unknown
	   E = enable
	   c = gfx code
	   x = x position
	   y = y position
	   C = color code
	   w = width
	   X = x flip
	   Y = y flip
	   * = alters y position in bluehawk and flytiger - see code below
	   bit 11 of gfx code only used by gulfstrm, pollux, bluehawk and flytiger
	   height only used by pollux, bluehawk and flytiger
	   x flip and y flip only used by pollux and flytiger */

	UINT8 const *const buffered_spriteram = m_spriteram->buffer();
	for (int offs = 0; offs < m_spriteram->bytes(); offs += 32)
	{
		int sx = buffered_spriteram[offs+3] | ((buffered_spriteram[offs+1] & 0x10) << 4);
		int sy = buffered_spriteram[offs+2];
		int code = buffered_spriteram[offs] | ((buffered_spriteram[offs+1] & 0xe0) << 3);
		int color = buffered_spriteram[offs+1] & 0x0f;

		//TODO: This priority mechanism works for known games, but seems a bit strange.
		//Are we missing something?  (The obvious spare palette bit isn't it.)
		int const pri = (((color == 0x00) || (color == 0x0f)) ? 0xfc : 0xf0);

		bool flipx = false, flipy = false;
		int height = 0;
		if (extensions)
		{
			UINT8 const ext = buffered_spriteram[offs+0x1c];

			if (extensions & SPRITE_12BIT)
				code |= ((ext & 0x01) << 11);

			if (extensions & SPRITE_HEIGHT)
			{
				height = (ext & 0x70) >> 4;
				code &= ~height;

				flipx = ext & 0x08;
				flipy = ext & 0x04;
			}

			if (extensions & SPRITE_YSHIFT_BLUEHAWK)
				sy += 6 - ((~ext & 0x02) << 7);

			if (extensions & SPRITE_YSHIFT_FLYTIGER)
				sy -=(ext & 0x02) << 7;
		}

		if (flip_screen())
		{
			sx = 498 - sx;
			sy = 240 - (16 * height) - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		color |= m_palette_bank << 6;

		for (int y = 0; y <= height; y++)
		{
			m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
					code + y,
					color,
					flipx, flipy,
					sx, sy + (16 * (flipy ? (height - y) : y)),
					screen.priority(),
					pri, 15);
		}
	}
}


UINT32 dooyong_z80_ym2203_state::screen_update_lastday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	/* Text layer is offset on this machine */
	if (!flip_screen())
		m_tx_tilemap->set_scrolly(0, 8);
	else
		m_tx_tilemap->set_scrolly(0, -8);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	if (!m_sprites_disabled)
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}

UINT32 dooyong_z80_ym2203_state::screen_update_gulfstrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	/* Text layer is offset on this machine */
	if (!flip_screen())
		m_tx_tilemap->set_scrolly(0, 8);
	else
		m_tx_tilemap->set_scrolly(0, -8);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT);

	return 0;
}

UINT32 dooyong_z80_ym2203_state::screen_update_pollux(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT | SPRITE_HEIGHT);

	return 0;
}

UINT32 dooyong_z80_state::screen_update_flytiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	if (m_flytiger_pri)
	{
		m_fg->draw(screen, bitmap, cliprect, 0, 1);
		m_bg->draw(screen, bitmap, cliprect, 0, 2);
	}
	else
	{
		m_bg->draw(screen, bitmap, cliprect, 0, 1);
		m_fg->draw(screen, bitmap, cliprect, 0, 2);
	}
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT | SPRITE_HEIGHT | SPRITE_YSHIFT_FLYTIGER);

	return 0;
}


UINT32 dooyong_z80_state::screen_update_bluehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_fg2->draw(screen, bitmap, cliprect, 0, 4);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT | SPRITE_HEIGHT | SPRITE_YSHIFT_BLUEHAWK);

	return 0;
}

UINT32 dooyong_z80_state::screen_update_primella(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 0);
	if (m_tx_pri) m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg->draw(screen, bitmap, cliprect, 0, 0);
	if (!m_tx_pri) m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



VIDEO_START_MEMBER(dooyong_z80_ym2203_state, lastday)
{
	/* Configure tilemap callbacks */
	m_tx_tilemap_mode = 0;

	/* Create tilemaps */
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_z80_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 64, 32);

	/* Configure tilemap transparency */
	m_tx_tilemap->set_transparent_pen(15);

	/* Register for save/restore */
	save_item(NAME(m_sprites_disabled));
	save_item(NAME(m_interrupt_line_1));
	save_item(NAME(m_interrupt_line_2));
}

VIDEO_START_MEMBER(dooyong_z80_ym2203_state, gulfstrm)
{
	/* Configure tilemap callbacks */
	m_tx_tilemap_mode = 0;

	/* Create tilemaps */
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_z80_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 64, 32);

	/* Configure tilemap transparency */
	m_tx_tilemap->set_transparent_pen(15);

	m_palette_bank = 0;

	/* Register for save/restore */
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_interrupt_line_1));
	save_item(NAME(m_interrupt_line_2));
}

VIDEO_START_MEMBER(dooyong_z80_ym2203_state, pollux)
{
	/* Configure tilemap callbacks */
	m_tx_tilemap_mode = 0;

	m_paletteram_flytiger = make_unique_clear<UINT8[]>(0x1000);
	save_pointer(NAME(m_paletteram_flytiger.get()), 0x1000);

	/* Create tilemaps */
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_z80_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 64, 32);

	/* Configure tilemap transparency */
	m_tx_tilemap->set_transparent_pen(15);

	m_palette_bank = 0;

	/* Register for save/restore */
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_interrupt_line_1));
	save_item(NAME(m_interrupt_line_2));
}

VIDEO_START_MEMBER(dooyong_z80_state, bluehawk)
{
	/* Configure tilemap callbacks */
	m_tx_tilemap_mode = 1;

	/* Create tilemaps */
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_z80_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 64, 32);

	/* Configure tilemap transparency */
	m_tx_tilemap->set_transparent_pen(15);
}

VIDEO_START_MEMBER(dooyong_z80_state, flytiger)
{
	/* Configure tilemap callbacks */
	m_tx_tilemap_mode = 0;

	m_paletteram_flytiger = make_unique_clear<UINT8[]>(0x1000);
	save_pointer(NAME(m_paletteram_flytiger.get()), 0x1000);

	/* Create tilemaps */
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_z80_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 64, 32);

	/* Configure tilemap transparency */
	m_tx_tilemap->set_transparent_pen(15);

	m_palette_bank = 0;

	/* Register for save/restore */
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_flytiger_pri));
}

VIDEO_START_MEMBER(dooyong_z80_state, primella)
{
	/* Configure tilemap callbacks */
	m_tx_tilemap_mode = 1;

	/* Create tilemaps */
	m_tx_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_z80_state::get_tx_tile_info),this), TILEMAP_SCAN_COLS,
			8, 8, 64, 32);

	/* Configure tilemap transparency */
	m_tx_tilemap->set_transparent_pen(15);

	/* Register for save/restore */
	save_item(NAME(m_tx_pri));
}


WRITE16_MEMBER(dooyong_68k_state::ctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 flips screen */
		flip_screen_set(data & 0x0001);

		/* bit 4 changes tilemaps priority */
		m_bg2_priority = data & 0x0010;

		/* bit 5 used but unknown */
	}
}


inline void dooyong_68k_state::rshark_get_tile_info(tile_data &tileinfo, int tile_index,
		UINT8 const *tilerom1, UINT8 const *tilerom2, UINT8 const *scroll, int graphics)
{
		/* Tiles take two bytes in tile ROM 1:
		                 MSB   LSB
		   [offs + 0x00] YX?c cccc    (Y flip, X flip, bits 12-8 of gfx code)
		   [offs + 0x01] cccc cccc    (bits 7-0 of gfx code)
		   ? = unused/unknown
		   c = gfx code
		   X = x flip
		   Y = y flip */
	int const offs = tile_index + ((int)scroll[1] << 9);
	int const attr = tilerom1[offs * 2];
	int const code = tilerom1[(offs * 2) + 1] | ((attr & 0x1f) << 8);
	int const color = tilerom2[offs] & 0x0f;
	int const flags = TILE_FLIPYX((attr & 0xC0) >> 6);

	tileinfo.set(graphics, code, color, flags);
}

TILE_GET_INFO_MEMBER(dooyong_68k_state::rshark_get_bg_tile_info)
{
	rshark_get_tile_info(tileinfo, tile_index, m_bg_tilerom, m_bg_tilerom2, m_bgscroll8, m_bg_gfx);
}

TILE_GET_INFO_MEMBER(dooyong_68k_state::rshark_get_bg2_tile_info)
{
	rshark_get_tile_info(tileinfo, tile_index, m_bg2_tilerom, m_bg2_tilerom2, m_bg2scroll8, m_bg2_gfx);
}

TILE_GET_INFO_MEMBER(dooyong_68k_state::rshark_get_fg_tile_info)
{
	rshark_get_tile_info(tileinfo, tile_index, m_fg_tilerom, m_fg_tilerom2, m_fgscroll8, m_fg_gfx);
}

TILE_GET_INFO_MEMBER(dooyong_68k_state::rshark_get_fg2_tile_info)
{
	rshark_get_tile_info(tileinfo, tile_index, m_fg2_tilerom, m_fg2_tilerom2, m_fg2scroll8, m_fg2_gfx);
}


void dooyong_68k_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Sprites take 8 16-bit words each in memory:
	              MSB             LSB
	   [offs + 0] ???? ???? ???? ???E
	   [offs + 1] ???? ???? hhhh wwww
	   [offs + 2] ???? ???? ???? ????
	   [offs + 3] cccc cccc cccc cccc
	   [offs + 4] ???? ???x xxxx xxxx
	   [offs + 5] ???? ???? ???? ????
	   [offs + 6] ???? ???y yyyy yyyy
	   [offs + 7] ???? ???? ???? CCCC
	   ? = unused/unknown
	   E = enable
	   c = gfx code
	   x = x offset
	   y = y offset (signed)
	   C = color code
	   w = width
	   h = height */

	UINT16 const *const buffered_spriteram = m_spriteram->buffer();
	for (int offs = (m_spriteram->bytes() / 2) - 8; offs >= 0; offs -= 8)
	{
		if (buffered_spriteram[offs] & 0x0001)    /* enable */
		{
			int code = buffered_spriteram[offs+3];
			int const color = buffered_spriteram[offs+7] & 0x000f;
			//TODO: This priority mechanism works for known games, but seems a bit strange.
			//Are we missing something?  (The obvious spare palette bit isn't it.)
			int const pri = (((color == 0x00) || (color == 0x0f)) ? 0xfc : 0xf0);
			int const width = buffered_spriteram[offs+1] & 0x000f;
			int const height = (buffered_spriteram[offs+1] & 0x00f0) >> 4;

			bool const flip = flip_screen();
			int sx = buffered_spriteram[offs+4] & 0x01ff;
			int sy = (INT16)buffered_spriteram[offs+6] & 0x01ff;
			if (sy & 0x0100) sy |= ~(int)0x01ff;    // Correctly sign-extend 9-bit number
			if (flip)
			{
				sx = 498 - (16 * width) - sx;
				sy = 240 - (16 * height) - sy;
			}

			for (int y = 0; y <= height; y++)
			{
				int const _y = sy + (16 * (flip ? (height - y) : y));
				for (int x = 0; x <= width; x++)
				{
					int const _x = sx + (16 * (flip ? (width - x) : x));
					m_gfxdecode->gfx(0)->prio_transpen(bitmap,cliprect,
							code,
							color,
							flip, flip,
							_x, _y,
							screen.priority(),
							pri, 15);
					code++;
				}
			}
		}
	}
}

UINT32 dooyong_68k_state::screen_update_rshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, (m_bg2_priority ? 2 : 1));
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_fg2_tilemap->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

UINT32 dooyong_68k_state::screen_update_popbingo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg_bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg->draw(screen, m_bg_bitmap, cliprect, 0, 1);

	m_bg2_bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg2->draw(screen, m_bg2_bitmap, cliprect, 0, 1);

	for (int y = cliprect.min_y; cliprect.max_y >= y; y++)
	{
		UINT16 const *const bg_src(&m_bg_bitmap.pix16(y, 0));
		UINT16 const *const bg2_src(&m_bg2_bitmap.pix16(y, 0));
		UINT16 *const dst(&bitmap.pix16(y, 0));
		for (int x = cliprect.min_x; cliprect.max_x >= x; x++)
			dst[x] = 0x100U | (bg_src[x] << 4) | bg2_src[x];
	}

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}



VIDEO_START_MEMBER(dooyong_68k_state, rshark)
{
	/* Configure tilemap callbacks */
	m_bg_tilerom = memregion("gfx5")->base();
	m_bg2_tilerom = memregion("gfx4")->base();
	m_fg_tilerom = memregion("gfx3")->base();
	m_fg2_tilerom = memregion("gfx2")->base();
	m_bg_tilerom2 = memregion("gfx6")->base() + 0x60000;
	m_bg2_tilerom2 = memregion("gfx6")->base() + 0x40000;
	m_fg_tilerom2 = memregion("gfx6")->base() + 0x20000;
	m_fg2_tilerom2 = memregion("gfx6")->base() + 0x00000;
	m_bg_gfx = 4;
	m_bg2_gfx = 3;
	m_fg_gfx = 2;
	m_fg2_gfx = 1;

	/* Create tilemaps */
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_68k_state::rshark_get_bg_tile_info),this), TILEMAP_SCAN_COLS,
			16, 16, 64, 32);
	m_bg2_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_68k_state::rshark_get_bg2_tile_info),this), TILEMAP_SCAN_COLS,
			16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_68k_state::rshark_get_fg_tile_info),this), TILEMAP_SCAN_COLS,
			16, 16, 64, 32);
	m_fg2_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(dooyong_68k_state::rshark_get_fg2_tile_info),this), TILEMAP_SCAN_COLS,
			16, 16, 64, 32);

	/* Configure tilemap transparency */
	m_bg2_tilemap->set_transparent_pen(15);
	m_fg_tilemap->set_transparent_pen(15);
	m_fg2_tilemap->set_transparent_pen(15);

	clear_video_regs();

	/* Register for save/restore */
	save_item(NAME(m_bgscroll8));
	save_item(NAME(m_bg2scroll8));
	save_item(NAME(m_fgscroll8));
	save_item(NAME(m_fg2scroll8));
	save_item(NAME(m_bg2_priority));
}

VIDEO_START_MEMBER(dooyong_68k_state, popbingo)
{
	m_screen->register_screen_bitmap(m_bg_bitmap);
	m_screen->register_screen_bitmap(m_bg2_bitmap);

	clear_video_regs();

	/* Register for save/restore */
	save_item(NAME(m_fgscroll8)); // Not used atm
	save_item(NAME(m_fg2scroll8)); // Not used atm
	save_item(NAME(m_bg2_priority)); // Not used atm
}
