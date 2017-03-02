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

device_type const DOOYONG_ROM_TILEMAP = device_creator<dooyong_rom_tilemap_device>;
device_type const RSHARK_ROM_TILEMAP = device_creator<rshark_rom_tilemap_device>;
device_type const DOOYONG_RAM_TILEMAP = device_creator<dooyong_ram_tilemap_device>;


dooyong_tilemap_device_base::dooyong_tilemap_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			char const *shortname,
			char const *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_gfxnum(0)
	, m_tilemap(nullptr)
	, m_palette_bank(0)
{
}

void dooyong_tilemap_device_base::static_set_gfxdecode_tag(device_t &device, char const *tag)
{
	downcast<dooyong_tilemap_device_base &>(device).m_gfxdecode.set_tag(tag);
}

void dooyong_tilemap_device_base::static_set_gfxnum(device_t &device, int gfxnum)
{
	downcast<dooyong_tilemap_device_base &>(device).m_gfxnum = gfxnum;
}

void dooyong_tilemap_device_base::draw(screen_device &screen, bitmap_ind16 &dest, rectangle const &cliprect, uint32_t flags, uint8_t priority)
{
	m_tilemap->draw(screen, dest, cliprect, flags, priority);
}

void dooyong_tilemap_device_base::set_palette_bank(uint16_t bank)
{
	if (bank != m_palette_bank)
	{
		m_palette_bank = bank;
		m_tilemap->mark_all_dirty();
	}
}


dooyong_rom_tilemap_device::dooyong_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: dooyong_rom_tilemap_device(mconfig, DOOYONG_ROM_TILEMAP, "Dooyong ROM Tilemap", tag, owner, clock, "dooyong_rom_tilemap", __FILE__)
{
}

dooyong_rom_tilemap_device::dooyong_rom_tilemap_device(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			char const *shortname,
			char const *source)
	: dooyong_tilemap_device_base(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_rows(8)
	, m_tilerom(*this, finder_base::DUMMY_TAG)
	, m_tilerom_offset(0)
	, m_transparent_pen(~0U)
	, m_primella_code_mask(0x03ff)
	, m_primella_color_mask(0x3c00)
	, m_primella_color_shift(10)
	, m_registers{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
{
}

void dooyong_rom_tilemap_device::static_set_tilerom_tag(device_t &device, char const *tag)
{
	downcast<dooyong_rom_tilemap_device &>(device).m_tilerom.set_tag(tag);
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

WRITE8_MEMBER(dooyong_rom_tilemap_device::ctrl_w)
{
	offset &= 0x07U;
	uint8_t const old = m_registers[offset];
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

void dooyong_rom_tilemap_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(dooyong_rom_tilemap_device::tile_info), this),
			TILEMAP_SCAN_COLS,
			gfx().width(),
			gfx().height(),
			1024 / gfx().width(),
			m_rows);
	if (~m_transparent_pen)
		m_tilemap->set_transparent_pen(m_transparent_pen);

	if (0 > m_tilerom_offset)
		m_tilerom_offset = m_tilerom.length() + m_tilerom_offset;

	std::fill(std::begin(m_registers), std::end(m_registers), 0U);
	m_palette_bank = 0U;

	save_item(NAME(m_registers));
	save_item(NAME(m_palette_bank));
}

TILE_GET_INFO_MEMBER(dooyong_rom_tilemap_device::tile_info)
{
	unsigned const attr = m_tilerom[m_tilerom_offset + adjust_tile_index(tile_index)];
	unsigned code, color, flags;
	if (BIT(m_registers[6], 5))
	{   // lastday/gulfstrm/pollux/flytiger
		// Tiles take one word in ROM:
		// MSB             LSB
		// cCCC CYXc cccc cccc  (bit 9 of gfx code, bits 3-0 of color, Y flip, X flip, bits 8-0 of gfx code)
		// c = gfx code
		// C = color code
		// X = x flip
		// Y = y flip
		code = (BIT(attr, 15) << 9) | (attr & 0x01ff);
		color = m_palette_bank | ((attr >> 11) & 0x0fU);
		flags = TILE_FLIPYX((attr >> 9) & 0x03U);
	}
	else
	{   // primella/popbingo
		// Tiles take one word in ROM:
		//          MSB             LSB
		// primella YXCC CCcc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 9-0 of gfx code)
		// popbingo YX?? ?ccc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 10-0 of gfx code)
		// rshark   YX?c cccc cccc cccc (Y flip, X flip, bits 3-0 of color code, bits 12-0 of gfx code)
		// c = gfx code
		// C = color code
		// X = x flip
		// Y = y flip
		// ? = unused?
		color = m_palette_bank | ((attr & m_primella_color_mask) >> m_primella_color_shift);
		flags = TILE_FLIPYX((attr >> 14) & 0x03U);
		code = attr & m_primella_code_mask;
	}

	tileinfo.set(m_gfxnum, code, color, flags);
}


rshark_rom_tilemap_device::rshark_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: dooyong_rom_tilemap_device(mconfig, RSHARK_ROM_TILEMAP, "R-Shark ROM Tilemap", tag, owner, clock, "rshark_rom_tilemap", __FILE__)
	, m_colorrom(*this, finder_base::DUMMY_TAG)
	, m_colorrom_offset(0)
{
	m_rows = 32;
}

void rshark_rom_tilemap_device::static_set_colorrom_tag(device_t &device, char const *tag)
{
	downcast<rshark_rom_tilemap_device &>(device).m_colorrom.set_tag(tag);
}

void rshark_rom_tilemap_device::static_set_colorrom_offset(device_t &device, int offset)
{
	downcast<rshark_rom_tilemap_device &>(device).m_colorrom_offset = offset;
}

void rshark_rom_tilemap_device::device_start()
{
	dooyong_rom_tilemap_device::device_start();

	if (0 > m_colorrom_offset)
		m_colorrom_offset = m_colorrom.length() + m_colorrom_offset;
}

TILE_GET_INFO_MEMBER(rshark_rom_tilemap_device::tile_info)
{
	dooyong_rom_tilemap_device::tile_info(tilemap, tileinfo, tile_index);

	uint8_t const color = m_colorrom[m_colorrom_offset + adjust_tile_index(tile_index)] & 0x0fU;
	tileinfo.palette_base = gfx().colorbase() + (gfx().granularity() * (color % gfx().colors()));
}


dooyong_ram_tilemap_device::dooyong_ram_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: dooyong_tilemap_device_base(mconfig, DOOYONG_RAM_TILEMAP, "Dooyong RAM Tilemap", tag, owner, clock, "dooyong_ram_tilemap", __FILE__)
	, m_tileram()
{
}

WRITE16_MEMBER(dooyong_ram_tilemap_device::tileram_w)
{
	offset &= (64U * 32U) - 1U;
	uint16_t value(m_tileram[offset]);
	COMBINE_DATA(&value);
	if (value != m_tileram[offset])
	{
		m_tileram[offset] = value;
		m_tilemap->mark_tile_dirty(offset);
	}
}

void dooyong_ram_tilemap_device::device_start()
{
	if (!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(dooyong_ram_tilemap_device::tile_info), this),
			TILEMAP_SCAN_COLS,
			8,
			8,
			64,
			32);
	m_tilemap->set_transparent_pen(15);

	m_tileram.reset(new uint16_t[64 * 32]);
	std::fill(m_tileram.get(), m_tileram.get() + (64 * 32), 0U);
	m_palette_bank = 0U;

	save_pointer(NAME(m_tileram.get()), 64 * 32);
	save_item(NAME(m_palette_bank));
}

TILE_GET_INFO_MEMBER(dooyong_ram_tilemap_device::tile_info)
{
	// Each tile takes one word of memory:
	// MSB             LSB
	// CCCC cccc cccc cccc  (bits 3-0 of color code, bits 11-0 of gfx code)
	// c = gfx code
	// C = color code
	unsigned const attr(m_tileram[tile_index]);
	tileinfo.set(m_gfxnum, attr & 0x0fffU, m_palette_bank | ((attr >> 12) & 0x0fU), 0);
}


READ8_MEMBER(dooyong_z80_state::lastday_tx_r)
{
	bool const lane(BIT(offset, 11));
	return m_tx->tileram_r(space, offset & 0x07ffU) >> (lane ? 8 : 0);
}

WRITE8_MEMBER(dooyong_z80_state::lastday_tx_w)
{
	bool const lane(BIT(offset, 11));
	m_tx->tileram_w(space, offset & 0x07ffU, uint16_t(data) << (lane ? 8 : 0), lane ? 0xff00U : 0x00ffU);
}

READ8_MEMBER(dooyong_z80_state::bluehawk_tx_r)
{
	bool const lane(BIT(offset, 0));
	return m_tx->tileram_r(space, offset >> 1) >> (lane ? 8 : 0);
}

WRITE8_MEMBER(dooyong_z80_state::bluehawk_tx_w)
{
	bool const lane(BIT(offset, 0));
	m_tx->tileram_w(space, offset >> 1, uint16_t(data) << (lane ? 8 : 0), lane ? 0xff00U : 0x00ffU);
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
	uint8_t const last_palbank = m_palette_bank;
	if (m_paletteram_flytiger) m_palette_bank = BIT(data, 1);
	if (last_palbank != m_palette_bank)
	{
		m_bg->set_palette_bank(m_palette_bank << 6);
		m_fg->set_palette_bank(m_palette_bank << 6);
		m_tx->set_palette_bank(m_palette_bank << 6);
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
	uint16_t const value = m_paletteram_flytiger[offset & ~1] | (m_paletteram_flytiger[offset | 1] << 8);
	m_palette->set_pen_color(offset/2, pal5bit(value >> 10), pal5bit(value >> 5), pal5bit(value >> 0));

}

WRITE8_MEMBER(dooyong_z80_state::flytiger_ctrl_w)
{
	/* bit 0 is flip screen */
	flip_screen_set(data & 0x01);

	/* bits 1, 2 used but unknown */

	/* bit 3 palette banking  */
	uint8_t const last_palbank = m_palette_bank;
	m_palette_bank = BIT(data, 3);
	if (last_palbank != m_palette_bank)
	{
		m_bg->set_palette_bank(m_palette_bank << 6);
		m_fg->set_palette_bank(m_palette_bank << 6);
		m_tx->set_palette_bank(m_palette_bank << 6);
	}

	/* bit 4 changes tilemaps priority */
	m_flytiger_pri = data & 0x10;
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

	uint8_t const *const buffered_spriteram = m_spriteram->buffer();
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
			uint8_t const ext = buffered_spriteram[offs+0x1c];

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


uint32_t dooyong_z80_ym2203_state::screen_update_lastday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	/* Text layer is offset on this machine */
	m_tx->set_scrolly(flip_screen() ? -8 : 8);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_tx->draw(screen, bitmap, cliprect, 0, 4);

	if (!m_sprites_disabled)
		draw_sprites(screen, bitmap, cliprect);

	return 0;
}

uint32_t dooyong_z80_ym2203_state::screen_update_gulfstrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	/* Text layer is offset on this machine */
	m_tx->set_scrolly(flip_screen() ? -8 : 8);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_tx->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT);

	return 0;
}

uint32_t dooyong_z80_ym2203_state::screen_update_pollux(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_tx->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT | SPRITE_HEIGHT);

	return 0;
}

uint32_t dooyong_z80_state::screen_update_flytiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
	m_tx->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT | SPRITE_HEIGHT | SPRITE_YSHIFT_FLYTIGER);

	return 0;
}


uint32_t dooyong_z80_state::screen_update_bluehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_fg2->draw(screen, bitmap, cliprect, 0, 4);
	m_tx->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites(screen, bitmap, cliprect, SPRITE_12BIT | SPRITE_HEIGHT | SPRITE_YSHIFT_BLUEHAWK);

	return 0;
}

uint32_t dooyong_z80_state::screen_update_primella(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 0);
	if (m_tx_pri) m_tx->draw(screen, bitmap, cliprect, 0, 0);
	m_fg->draw(screen, bitmap, cliprect, 0, 0);
	if (!m_tx_pri) m_tx->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



VIDEO_START_MEMBER(dooyong_z80_ym2203_state, lastday)
{
	/* Register for save/restore */
	save_item(NAME(m_sprites_disabled));
	save_item(NAME(m_interrupt_line_1));
	save_item(NAME(m_interrupt_line_2));
}

VIDEO_START_MEMBER(dooyong_z80_ym2203_state, gulfstrm)
{
	m_palette_bank = 0;

	/* Register for save/restore */
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_interrupt_line_1));
	save_item(NAME(m_interrupt_line_2));
}

VIDEO_START_MEMBER(dooyong_z80_ym2203_state, pollux)
{
	m_paletteram_flytiger = make_unique_clear<uint8_t[]>(0x1000);
	save_pointer(NAME(m_paletteram_flytiger.get()), 0x1000);

	m_palette_bank = 0;

	/* Register for save/restore */
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_interrupt_line_1));
	save_item(NAME(m_interrupt_line_2));
}

VIDEO_START_MEMBER(dooyong_z80_state, bluehawk)
{
}

VIDEO_START_MEMBER(dooyong_z80_state, flytiger)
{
	m_paletteram_flytiger = make_unique_clear<uint8_t[]>(0x1000);
	save_pointer(NAME(m_paletteram_flytiger.get()), 0x1000);

	m_palette_bank = 0;

	/* Register for save/restore */
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_flytiger_pri));
}

VIDEO_START_MEMBER(dooyong_z80_state, primella)
{
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

	uint16_t const *const buffered_spriteram = m_spriteram->buffer();
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
			int sy = (int16_t)buffered_spriteram[offs+6] & 0x01ff;
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


uint32_t rshark_state::screen_update_rshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg->draw(screen, bitmap, cliprect, 0, 1);
	m_bg2->draw(screen, bitmap, cliprect, 0, (m_bg2_priority ? 2 : 1));
	m_fg->draw(screen, bitmap, cliprect, 0, 2);
	m_fg2->draw(screen, bitmap, cliprect, 0, 2);

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

VIDEO_START_MEMBER(rshark_state, rshark)
{
	/* Register for save/restore */
	save_item(NAME(m_bg2_priority));
}


uint32_t popbingo_state::screen_update_popbingo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);

	m_bg_bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg->draw(screen, m_bg_bitmap, cliprect, 0, 1);

	m_bg2_bitmap.fill(m_palette->black_pen(), cliprect);
	m_bg2->draw(screen, m_bg2_bitmap, cliprect, 0, 1);

	for (int y = cliprect.min_y; cliprect.max_y >= y; y++)
	{
		uint16_t const *const bg_src(&m_bg_bitmap.pix16(y, 0));
		uint16_t const *const bg2_src(&m_bg2_bitmap.pix16(y, 0));
		uint16_t *const dst(&bitmap.pix16(y, 0));
		for (int x = cliprect.min_x; cliprect.max_x >= x; x++)
			dst[x] = 0x100U | (bg_src[x] << 4) | bg2_src[x];
	}

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

VIDEO_START_MEMBER(popbingo_state, popbingo)
{
	m_screen->register_screen_bitmap(m_bg_bitmap);
	m_screen->register_screen_bitmap(m_bg2_bitmap);

	/* Register for save/restore */
	save_item(NAME(m_bg2_priority)); // Not used atm
}
