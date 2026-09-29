// license:BSD-3-Clause
// copyright-holders:David Haywood, Andrea Bogazzi

// MCU simulation code for IREM's Beyond Kung Fu

#include "emu.h"
#include "m62_bkungfu_mcu.h"

/*

TODO:
- determine the purpose of the optional fifth/sixth level composition pointers
- model MCU execution timing rather than using high-level synchronous command handlers
- test mode doesn't work (there are strings for it in the MCU data ROM, is the MCU involved?)

NOTES ON MCU DATA ROM FORMAT
----------------------------

command 0x01 at offset 0x00 uses the table at 0x200

The Z80 parameter is the direct table index and advances modulo eight in the
order 1,2,3,4,5,6,7,0.  Consequently slot 0 below is gameplay stage 8, slots
1-7 are gameplay stages 1-7, and redraw slots 8-15 have the same ordering.
Older notes below use "Stage 1-8" as shorthand for table slots 0-7.

Table for levels, initial state
0200  4D 18 | 184d
0202  DD 17 | 17dd
0204  6D 17 | 176d
0206  FD 16 | 16fd
0208  8D 16 | 168d
020A  1D 16 | 161d
020C  AD 15 | 15ad
020E  3D 15 | 153d

Tables for levels, state for redrawing with animated pieces already moved (called after pieces have moved, or after respawning on death)
0210  4D 18 | 184d
0212  DD 17 | 17dd
0214  6D 17 | 176d
0216  7D 1A | 1a7d
0218  0D 1A | 1a0d
021A  9D 19 | 199d
021C  2D 19 | 192d
021E  BD 18 | 18bd

This initial / redraw after animation table use can be confirmed by looking at the pairs

0200  4D 18 | 184d / 0210  4D 18 | 184d  - identical in both states (Stage 1 data)
0202  DD 17 | 17dd / 0212  DD 17 | 17dd  - identical in both states (Stage 2 data)
0204  6D 17 | 176d / 0214  6D 17 | 176d  - identical in both states (Stage 3 data)
0206  FD 16 | 16fd / 0216  7D 1A | 1a7d  - different (Stage 4 data)
0208  8D 16 | 168d / 0218  0D 1A | 1a0d  - different (Stage 5 data)
020A  1D 16 | 161d / 021A  9D 19 | 199d  - different (Stage 6 data)
020C  AD 15 | 15ad / 021C  2D 19 | 192d  - different (Stage 7 data)
020E  3D 15 | 153d / 021E  BD 18 | 18bd  - different (Stage 8 data)

The game has 8 stages, the first 3 stages do not contain animated objects
The remaining stages have animated objects (animated with different commands) that close behind the player when they first enter the stage

Stage 4 contains a door on the very right of the tilemap
Stage 5 contains a trap door on the very left of the tilemap
Stage 6 contains a trap door on the very right of the tilemap
Stage 7 contains a trap door on the very left of the tilemap
Stage 8 contains a trap door on the very right of the tilemap

Due to this you would expect the data pointed to by the stages with tiny modifications to be similar once decrypted as only a few details
are different between the 2 versions so
16fd should be similar to 1a7d
168d should be similar to 1a0d
161d should be similar to 199d
15ad should be similar to 192d
153d should be similar to 18bd

if you sort by address pointed to you get

020E  3D 15 | 153d
020C  AD 15 | 15ad
020A  1D 16 | 161d
0208  8D 16 | 168d
0206  FD 16 | 16fd
0204  6D 17 | 176d / 0214  6D 17 | 176d
0202  DD 17 | 17dd / 0212  DD 17 | 17dd
0200  4D 18 | 184d / 0210  4D 18 | 184d
021E  BD 18 | 18bd
021C  2D 19 | 192d
021A  9D 19 | 199d
0218  0D 1A | 1a0d
0216  7D 1A | 1a7d

so each of these blocks is 0x70 bytes long giving the following ranges

153d - 15ac
15ad - 161c
161d - 168c
168d - 16fc
16fd - 176c
176d - 17dc
17dd - 184c
184d - 18bc
18bd - 192c
192d - 199c
199d - 1a0d
1a0d - 1a7d
1a7d - 1aec

data from 1aed - 7fff is not directly referenced by anything, so the tables above probably point to it

when the levels are drawn they're drawn in 4 tile wide strips, from top to bottom, left to right
each screen has 8 of these strips
each level is 7 screens wide

as the level structures above are each 0x70 bytes wide, this likely means that each screen takes up
0x10 bytes of that structure, so 8x2 byte pointers to the later data structures, 1 for each strip

several other games in m62.cpp also draw their backgrounds in 4 tile wide strips

Additional notes from Andrea Bogazzi:
-------------------------------------

Later traces of a real one-player start resolved the table numbering above.
The Z80 sends index 00 for the initial Precinct layout, then index 08 for its
corresponding post-intro/redraw layout.  Therefore 00-07 are the initial states
for gameplay stages 1-8 and 08-0f are their matching redraw states.  The older
1,2,3,4,5,6,7,0 interpretation came from following the attract-mode path.

All multi-byte values described below are little-endian.  Bytes at ROM offsets
0000-153c are plaintext.  Starting at 153d, each byte is independently decoded
with a 256-byte key table K.  For ROM address A:

    s = (A_low + A_high) & ff
    first = (s & 1) ? (K[s] - cipher) : (cipher ^ K[s])

This transform is address-local: it has no feedback from preceding bytes and
can be performed as data is read.  No shorter generator for K is known.

Level/object payload bytes and the pointers stored inside those payloads use a
second address-dependent transform after the first one:

    if (s & 1)
        value = 60 - first
    else
        value = ((first & 20) ? a0 : 60) - first
                - ((first & 1) ? 0 : 2)

All arithmetic is modulo 256.  Structure tokens are recognised at the stage
used by their format, rather than blindly applying the payload transform to
every byte.

The level directory at 0200 contains sixteen pointers.  Each points to a
70-byte block of 56 two-byte entry pointers.  Consecutive entry pairs describe
two adjacent four-tile-wide columns: the even entry supplies the upper ten
tile rows and the odd entry supplies the lower sixteen rows.  Each entry points
to a 0000-terminated list of payload-encoded pointers.  The first four select
the tile streams; optional fifth and sixth pointers select unresolved seven-
byte records associated with the six boundaries between the seven screens.
Each main stream supplies two adjacent tiles per row; four streams therefore
form the eight tiles across the column pair.  Upper streams contain 20 literal
cells and lower streams contain 32.  In a level stream, 00 terminates the
stream, 01 followed by a payload byte changes the current attribute, and all
other payload bytes are tile codes.  The current attribute is written beside
every tile code in tilemap RAM.

Object IDs 80-90 select seventeen pointers in the table at 0100.  The selected
five-byte record is:

    width, destination_low, destination_high, stream_low, stream_high

The object stream is row-major and wraps after the record's width.  Token 5f
is followed by a payload-encoded attribute; 5e or 60 terminates the stream;
other bytes are payload-encoded tile codes.  This same format describes both
title dragons, both flame animation frames and the later-stage moving objects.

ROM text commands use a two-byte pointer table indexed by the command's text
number.  Text streams use 00 as terminator, 01 followed by an attribute, 02
followed by a little-endian tilemap position, and literal tile bytes otherwise.
The fixed HUD uses the same grammar through the pointer at 0140.  Command 0c
uses an equivalent stream prepared by the Z80 in shared RAM.  Text positions
advance by two bytes per character and wrap within a 64-tile (0x80-byte) row.

The data format contains the tile codes and palette attributes.  In tilemap
RAM they are emitted as adjacent bytes, tile first and attribute second.  The
tile byte supplies code bits 0-7; attribute bits 5-7 supply code bits 8-10 and
attribute bits 0-4 select the palette.  The separate background-bank latch
supplies code bit 11.  The solid-colour low tile codes 04-0b in each bank are
ordinary tiles aligned with the palette colours; they do not require a special
fill opcode.

*/


DEFINE_DEVICE_TYPE(BKUNG_MCU, bkungfu_mcu_device, "bkung_mcu", "Irem Beyond Kung-Fu MCU")

bkungfu_mcu_device::bkungfu_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BKUNG_MCU, tag, owner, clock)
	, m_vram_w(*this)
	, m_level_vram_w(*this)
	, m_mailbox_out_w(*this)
{
}

void bkungfu_mcu_device::device_start()
{
	save_item(NAME(m_mailbox));
	save_item(NAME(m_timer));
	save_item(NAME(m_p1score));
	save_item(NAME(m_topscore));
	save_item(NAME(m_p2score));
	save_item(NAME(m_lives));
	save_item(NAME(m_player_energy));
	save_item(NAME(m_boss_energy));
	save_item(NAME(m_floorcount));
	save_item(NAME(m_floorcount_state));
	save_item(NAME(m_valid));
	save_item(NAME(m_initialized));
	save_item(NAME(m_running));
	save_item(NAME(m_leveldraw_row));
	save_item(NAME(m_leveldraw_column));
	save_item(NAME(m_leveldraw_number));
	m_leveldraw_timer = timer_alloc(FUNC(bkungfu_mcu_device::leveldraw_next), this);
}

void bkungfu_mcu_device::device_reset()
{
	std::fill(std::begin(m_mailbox), std::end(m_mailbox), 0);
	m_timer = 0;
	m_p1score = 0;
	m_topscore = 0;
	m_p2score = 0;
	m_lives = 0;
	m_player_energy = 0;
	m_boss_energy = 0;
	m_floorcount = 0;
	m_floorcount_state = 0;
	m_valid = 0;
	m_initialized = false;
	m_running = false;
	m_leveldraw_row = 0;
	m_leveldraw_column = 0;
	m_leveldraw_number = 0;
	m_leveldraw_timer->adjust(attotime::never);
}

void bkungfu_mcu_device::clear()
{
	m_initialized = false;
}

void bkungfu_mcu_device::set_data_rom(const uint8_t *data_rom)
{
	m_data_rom = data_rom;
}

uint8_t bkungfu_mcu_device::decrypt_data(uint16_t address) const
{
    if (!m_data_rom || address >= 0x8000)
        return 0xff;

    uint8_t const cipher = m_data_rom[address];
    if (address < 0x153d)
        return cipher;

    uint8_t const index = uint8_t((address & 0xff) + (address >> 8));

    if (index & 1)
        return uint8_t(
            (index ^ ((index & 2) ? 0x7e : 0x12)) - 0x20 - cipher);

    return uint8_t(cipher ^ index ^ ((index & 2) ? 0x5e : 0xae));
}

void bkungfu_mcu_device::write_number(int x, int y, uint8_t number)
{
	m_vram_w(((y * 0x40 + x) << 1) & 0x0fff, (number & 0x0f) + 0x30);
}

void bkungfu_mcu_device::write_floor_dot(int which, bool lit)
{
	m_vram_w(((3 * 0x40 + 0x20 + which * 2) << 1) & 0x0fff, lit ? 0xd5 : 0xd6);
}

void bkungfu_mcu_device::write_lifebar(int xbase, int ybase, uint8_t energy, bool boss)
{
	int const full_segments = (energy & 0x78) >> 3;
	for (int segment = 0; segment < 8; segment++)
	{
		uint8_t const part = segment < full_segments ? 8 : segment == full_segments ? energy & 7 : 0;
		uint8_t const tile = part ? uint8_t((boss ? 0xcc : 0xc4) + 8 - part) : 0xc2;
		m_vram_w(((ybase * 0x40 + xbase + segment) << 1) & 0x0fff, tile);
	}
}

void bkungfu_mcu_device::update_slot(uint8_t slot)
{
	if (!m_initialized)
		return;

	switch (slot)
	{
	case 0x10:
		for (int i = 0; i < 6; i++) write_number(0x14 + i, 0, (m_p1score >> ((5 - i) * 4)) & 0x0f);
		break;
	case 0x14:
		for (int i = 0; i < 6; i++) write_number(0x29 + i, 0, (m_p2score >> ((5 - i) * 4)) & 0x0f);
		break;
	case 0x18:
		for (int i = 0; i < 6; i++) write_number(0x1f + i, 0, (m_topscore >> ((5 - i) * 4)) & 0x0f);
		break;
	case 0x1c:
		for (int i = 0; i < 4; i++) write_number(0x25 + i, 5, (m_timer >> ((3 - i) * 4)) & 0x0f);
		break;
	case 0x20:
		for (int i = 0; i < 8; i++) write_floor_dot(i, i <= (int(m_floorcount) - (m_floorcount_state == 0x02)));
		break;
	case 0x24:
		write_number(0x2d, 5, m_lives);
		break;
	case 0x28:
		if (m_player_energy <= 0x40) write_lifebar(0x17, 2, m_player_energy, false);
		break;
	case 0x2c:
		if (m_boss_energy <= 0x40) write_lifebar(0x17, 4, m_boss_energy, true);
		break;
	}
}

void bkungfu_mcu_device::complete(uint16_t offset)
{
	mailbox_out(offset, 0xfe);
}

void bkungfu_mcu_device::mailbox_out(uint16_t offset, uint8_t data)
{
	m_mailbox[offset] = data;
	m_mailbox_out_w(offset, data);
}

void bkungfu_mcu_device::draw_text(uint16_t table_offset, bool use_mailbox)
{
	if (!m_running || !m_data_rom)
		return;

	uint16_t data_address;
	const uint8_t *data;
	uint8_t position_low;
	uint8_t position_high;
	uint8_t attribute;
	if (use_mailbox)
	{
		data_address = 0x100;
		data = m_mailbox;
		position_low = 3;
		position_high = 4;
		attribute = 5;
	}
	else
	{
		data_address = m_data_rom[table_offset] | (uint16_t(m_data_rom[table_offset + 1]) << 8);
		data = m_data_rom;
		position_low = 2;
		position_high = 3;
		attribute = 4;
	}

	for (uint8_t value = data[data_address++]; value != 0; value = data[data_address++])
	{
		if (value == 0x01)
		{
			mailbox_out(attribute, data[data_address++]);
		}
		else if (value == 0x02)
		{
			mailbox_out(position_low, data[data_address++]);
			mailbox_out(position_high, data[data_address++]);
		}
		else
		{
			uint16_t position = (uint16_t(m_mailbox[position_high]) << 8) | m_mailbox[position_low];
			m_vram_w(position & 0x0fff, value);
			m_vram_w((position + 1) & 0x0fff, m_mailbox[attribute]);
			position = (position & ~0x007f) | ((position + 2) & 0x007f);
			mailbox_out(position_low, position & 0xff);
			mailbox_out(position_high, position >> 8);
		}
	}
}

void bkungfu_mcu_device::draw_credits_continue()
{
	if (!m_running)
		return;

	uint16_t const position = (uint16_t(m_mailbox[3]) << 8) | m_mailbox[2];
	uint8_t const attribute = m_mailbox[4];
	m_vram_w(position & 0x0fff, (m_mailbox[1] >> 4) + 0x30);
	m_vram_w((position + 1) & 0x0fff, attribute);
	m_vram_w((position + 2) & 0x0fff, (m_mailbox[1] & 0x0f) + 0x30);
	m_vram_w((position + 3) & 0x0fff, attribute);
}

void bkungfu_mcu_device::clear_tilemap()
{
	if (!m_running)
		return;

	for (uint16_t position = 0; position < 0x1000; position += 2)
	{
		m_vram_w(position, m_mailbox[2]);
		m_vram_w(position + 1, m_mailbox[1]);
	}
}

void bkungfu_mcu_device::execute_slot(uint8_t slot)
{
	uint8_t const trigger = m_mailbox[slot];
	uint8_t const p1 = m_mailbox[slot + 1];
	uint8_t const p2 = m_mailbox[slot + 2];
	uint8_t const p3 = m_mailbox[slot + 3];
	switch (slot)
	{
	case 0x10: m_p1score = (uint32_t(p1) << 16) | (uint32_t(p2) << 8) | p3; break;
	case 0x14: m_p2score = (uint32_t(p1) << 16) | (uint32_t(p2) << 8) | p3; break;
	case 0x18: m_topscore = (uint32_t(p1) << 16) | (uint32_t(p2) << 8) | p3; break;
	case 0x1c: m_timer = (uint16_t(p1) << 8) | p2; break;
	case 0x20: m_floorcount = p1; m_floorcount_state = trigger; break;
	case 0x24: m_lives = p1; break;
	case 0x28: m_player_energy = p1; break;
	case 0x2c: m_boss_energy = p1; break;
	default: return;
	}
	m_valid |= uint8_t(1U << ((slot - 0x10) >> 2));
	update_slot(slot);
	complete(slot);
}

u8 bkungfu_mcu_device::mailbox_r(offs_t offset)
{
	return m_mailbox[offset];
}

void bkungfu_mcu_device::mailbox_w(offs_t offset, uint8_t data)
{
	if (offset >= std::size(m_mailbox))
		return;

	m_mailbox[offset] = data;
	if (m_running && offset >= 0x10 && offset < 0x30 && !(offset & 3))
		execute_slot(offset);
}

uint8_t bkungfu_mcu_device::decode_payload(uint16_t address) const
{
	uint8_t const value = decrypt_data(address);
	uint8_t const sum = uint8_t((address & 0xff) + (address >> 8));
	if (sum & 1)
		return uint8_t(0x60 - value);

	uint8_t const base = (value & 0x20) ? 0xa0 : 0x60;
	return uint8_t(base - value - ((value & 1) ? 0 : 2));
}

uint16_t bkungfu_mcu_device::decode_payload_word(uint16_t address) const
{
	return decode_payload(address) | (uint16_t(decode_payload(address + 1)) << 8);
}

void bkungfu_mcu_device::draw_object(uint8_t id)
{
	if (!m_data_rom || id < 0x80 || id > 0x90)
		return;

	uint16_t const recaddr = decrypt_data(0x100 + 2 * (id - 0x80)) | (uint16_t(decrypt_data(0x100 + 2 * (id - 0x80) + 1)) << 8);
	if (recaddr == 0 || recaddr >= 0x8000 - 5)
		return;

	uint8_t const width = decrypt_data(recaddr);
	uint16_t const pos = (decrypt_data(recaddr + 1) | (uint16_t(decrypt_data(recaddr + 2)) << 8)) & 0xfff;
	uint16_t dataptr = decrypt_data(recaddr + 3) | (uint16_t(decrypt_data(recaddr + 4)) << 8);
	if (width == 0 || width > 0x20 || dataptr >= 0x8000)
		return;

	uint8_t attrs[512], tiles[512];
	int count = 0;
	uint8_t attr = 0;
	while (dataptr < 0x8000 && count < 512)
	{
		uint16_t const address = dataptr;
		uint8_t const value = decrypt_data(dataptr++);
		if (value == 0x5f)
		{
			if (dataptr >= 0x8000)
				break;
			attr = decode_payload(dataptr++);
			continue;
		}
		if (value == 0x5e || value == 0x60)
			break;
		tiles[count] = decode_payload(address);
		attrs[count] = attr;
		count++;
	}

	for (int cell = 0; cell < count; cell++)
	{
		int const column = cell % width;
		int const row = cell / width;
		uint16_t const destination = (pos + row * 0x80 + column * 2) & 0xfff;
		m_vram_w(destination, tiles[cell]);
		m_vram_w(destination + 1, attrs[cell]);
	}
}

void bkungfu_mcu_device::draw_level_column_row(int column, int row, uint8_t tile, uint8_t attr)
{
	int const offset = ((row + 6) * 256 + column * 4) * 2;
	for (int x = 0; x < 4; x++)
	{
		m_level_vram_w(offset + x * 2, tile);
		m_level_vram_w(offset + x * 2 + 1, attr);
	}
}

void bkungfu_mcu_device::draw_level_strip(int column, int row)
{
	if (!m_data_rom)
		return;

	draw_level_column_row(column, row, 0x05, 0x19);
	int const source_entry = (column & ~1) + ((row >= 10) ? 1 : 0);
	int const source_row = (row < 10)
		? row + ((column & 1) ? 10 : 0)
		: row - 10 + ((column & 1) ? 16 : 0);
	uint16_t const table = 0x200 + ((m_leveldraw_number & 0x0f) << 1);
	uint16_t const block = decrypt_data(table) | (uint16_t(decrypt_data(table + 1)) << 8);
	uint16_t const entry = block + source_entry * 2;
	if (block < 0x153d || entry >= 0x8000 - 1)
		return;

	uint16_t const record = decode_payload_word(entry);
	if (record < 0x1aed || record >= 0x8000 - 8)
		return;

	uint8_t tiles[4][64];
	uint8_t attrs[4][64];
	int count = -1;
	for (int stream = 0; stream < 4; stream++)
	{
		uint16_t dataptr = decode_payload_word(record + stream * 2);
		if (dataptr < 0x2349 || dataptr >= 0x8000)
			return;

		uint8_t attr = 0;
		int streamcount = 0;
		while (dataptr < 0x8000 && streamcount < 64)
		{
			uint8_t const value = decode_payload(dataptr++);
			if (value == 0x00)
				break;
			if (value == 0x01)
			{
				if (dataptr >= 0x8000)
					return;
				attr = decode_payload(dataptr++);
				continue;
			}
			tiles[stream][streamcount] = value;
			attrs[stream][streamcount] = attr;
			streamcount++;
		}

		if (count < 0)
			count = streamcount;
		else if (count != streamcount)
			return;
	}

	int const expected_count = (source_entry & 1) ? 32 : 20;
	if (count != expected_count)
		return;

	int const halfheight = count / 2;
	int const band = source_row / halfheight;
	int const bandrow = source_row % halfheight;
	int const offset = ((row + 6) * 256 + column * 4) * 2;
	for (int x = 0; x < 4; x++)
	{
		int const stream = band * 2 + x / 2;
		int const index = bandrow * 2 + x % 2;
		m_level_vram_w(offset + x * 2, tiles[stream][index]);
		m_level_vram_w(offset + x * 2 + 1, attrs[stream][index]);
	}
}

TIMER_CALLBACK_MEMBER(bkungfu_mcu_device::leveldraw_next)
{
	draw_level_strip(m_leveldraw_column, m_leveldraw_row);
	if (++m_leveldraw_row == 26)
	{
		m_leveldraw_row = 0;
		m_leveldraw_column++;
	}
	if (m_leveldraw_column != 0x38)
		m_leveldraw_timer->adjust(attotime::from_usec(LEVEL_DRAW_STEP_USEC));
	else
		complete(0);
}

void bkungfu_mcu_device::command_w(uint8_t command)
{
	if (command == 0xfe)
	{
		m_running = true;
		for (uint8_t slot = 0x10; slot <= 0x2c; slot += 4)
			complete(slot);
		complete(0x102);
		complete(0x106);
		complete(0x118);
		complete(0x11c);
		complete(0);
		return;
	}
	if (command == 0x08)
	{
		clear_tilemap();
		clear();
		complete(0);
		return;
	}
	if (command == 0x14 || command == 0x0d)
	{
		draw_text(uint16_t(m_mailbox[1]) << 1, false);
		complete(0);
		return;
	}
	if (command == 0x0c)
	{
		draw_text(0, true);
		complete(0);
		return;
	}
	if (command == 0x10)
	{
		draw_credits_continue();
		complete(0);
		return;
	}
	if (command == 0x05)
	{
		// Observed after command 0x0f for later-stage animation objects.
		// Its additional effect, if any, is not known yet.
		complete(0);
		return;
	}
	if (command == 0x01)
	{
		m_leveldraw_number = m_mailbox[1];
		complete(0);
		return;
	}
	if (command == 0x02)
	{
		m_leveldraw_row = 0;
		m_leveldraw_column = 0;
		m_leveldraw_timer->adjust(attotime::from_usec(LEVEL_DRAW_STEP_USEC));
		return;
	}
	if (command == 0x0f)
	{
		draw_object(m_mailbox[1]);
		complete(0);
		return;
	}
	if (command != 0x0a)
		return;

	// Pre-level setup: decode the ROM-described base HUD.  This is an
	// explicit MCU command, not a completion synthesized by the Z80 driver.
	if (!m_data_rom)
		return;

	uint16_t stream = m_data_rom[0x140] | (uint16_t(m_data_rom[0x141]) << 8);
	uint16_t position = 0;
	uint8_t attribute = 0;
	for (;;)
	{
		uint8_t const value = m_data_rom[stream++];
		if (value == 0x00)
			break;
		if (value == 0x01)
		{
			attribute = m_data_rom[stream++];
			continue;
		}
		if (value == 0x02)
		{
			uint8_t const low = m_data_rom[stream++];
			uint8_t const high = m_data_rom[stream++];
			position = low | (uint16_t(high) << 8);
			continue;
		}
		m_vram_w(position & 0x0fff, value);
		m_vram_w((position + 1) & 0x0fff, attribute);
		position = (position & ~0x007f) | ((position + 2) & 0x007f);
	}

	m_initialized = true;
	for (uint8_t slot = 0x10; slot <= 0x2c; slot += 4)
		if (m_valid & (1U << ((slot - 0x10) >> 2)))
			update_slot(slot);
	complete(0);
}
