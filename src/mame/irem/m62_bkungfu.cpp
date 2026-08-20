// license:GPL-2.0+
// copyright-holders:Jonas Jago
/*******************************************************************************

Beyond Kung-Fu

Irem M62-based unreleased Kung-Fu Master sequel
video reference: https://www.youtube.com/watch?v=Efr9EQkbCSQ

TODO:
- level backgrounds (encrypted tilemap data in MCU data ROM) (needs decryption)
- other background elements (title screen animation, moving elements in levels) (either encrypted data in MCU data ROM, or internal to MCU)
- remaining HUD elements (floor counter, continue screen, credit counter, high score table) again all handled by the MCU
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


*******************************************************************************/

#include "emu.h"
#include "m62.h"
#include "iremipt.h"

#include "machine/timer.h"

namespace {

class m62_bkungfu_state : public m62_state
{
public:
	m62_bkungfu_state(const machine_config &mconfig, device_type type, const char *tag)
		: m62_state(mconfig, type, tag)
		, m_bkungfu_tileram(*this, "tileram", 256*32*2, ENDIANNESS_LITTLE)
		, m_blitterdatarom(*this, "blitterdat")
	{ }

	void bkungfu(machine_config& config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t bkungfu_blitter_r(offs_t offset);
	void bkungfu_blitter_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bkungfu_bg_tile_info);
	DECLARE_VIDEO_START(bkungfu);

	void bkungfu_blitter_draw_4_tile_column_row(int column, int row, uint8_t tile, uint8_t attr);
	void bkungfu_blitter_draw_4_tile_column(int column, int row);
	void bkungfu_blitter_tilemap_w(uint16_t offset, uint8_t data);
	void bkungfu_blitter_draw_text_inner(uint16_t blitterromptr, bool use_ram);
	void bkungfu_blitter_draw_text();
	void bkungfu_blitter_clear_tilemap();
	uint8_t bkungfu_blitter_decode_payload(uint16_t address) const;
	uint16_t bkungfu_blitter_decode_payload_word(uint16_t address) const;
	void bkungfu_blitter_draw_object(uint8_t id);
	void bkungfu_blitter_set_number(int x, int y, uint8_t num);
	void bkungfu_blitter_set_player_energy(int x, int y, uint8_t num, bool is_boss);
	void bkungfu_blitter_set_floor_state(int which, int state);
	void bkungfu_blitter_draw_lifebar(int xbase, int ybase, uint8_t energy, bool is_boss);
	void bkungfu_blitter_draw_credits_continue();
	void redraw_hud();

	TIMER_CALLBACK_MEMBER(leveldraw_next);

	// done this way so it can be viewed win the debugger with save state registration
	uint8_t m_blittercmdram[0x800];

	uint16_t m_hud_timer;
	uint32_t m_hud_p1score;
	uint32_t m_hud_topscore;
	uint32_t m_hud_p2score;
	uint8_t m_hud_lives;
	uint8_t m_hud_player_energy;
	uint8_t m_hud_boss_energy;
	uint8_t m_hud_floorcount;
	uint8_t m_hud_floorcount_state;
	uint8_t m_leveldraw_row;
	uint8_t m_leveldraw_column;
	uint8_t m_leveldraw_number;

	int m_mcu_running;

	emu_timer *m_leveldraw_timer = nullptr;

	memory_share_creator<uint8_t> m_bkungfu_tileram;

	required_region_ptr<uint8_t> m_blitterdatarom;
	std::unique_ptr<uint8_t []> m_blitterdecrypted;
};



/*******************************************************************************
    Video
*******************************************************************************/

TILE_GET_INFO_MEMBER(m62_bkungfu_state::get_bkungfu_bg_tile_info)
{
	int code = m_bkungfu_tileram[(tile_index << 1)];
	int color = m_bkungfu_tileram[(tile_index << 1) | 1];

	tileinfo.set(0, code | ((color & 0xe0) << 3) | (m_kidniki_background_bank << 11), color & 0x1f, 0);

	// The title flame uses palette pair 0x0d and is visibly behind the player
	// sprite in the reference footage.  Beyond Kung-Fu therefore has the M62
	// tile-priority threshold strapped one step above Kung-Fu Master: only
	// palette pairs 0x0e-0x0f (colors 0x1c-0x1f) cover sprites.
	if ((tile_index / 256) < 6 || ((color & 0x1f) >> 1) > 0x0d)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;
}

VIDEO_START_MEMBER(m62_bkungfu_state,bkungfu)
{
	m62_start(tilemap_get_info_delegate(*this, FUNC(m62_bkungfu_state::get_bkungfu_bg_tile_info)), 32, 0, 8, 8, 256, 32);
}



/*******************************************************************************
    Blitter
*******************************************************************************/

uint8_t m62_bkungfu_state::bkungfu_blitter_r(offs_t offset)
{
	// this will read the various trigger addresses, checking if they're 0xfe
	// presumably this is written by the MCU to signal the task has been completed

	// read address is 0x00 for most commands
	// 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c for the 'HUD' commands

	// it also checks 0102, 0106, 0118, 011c before sending command 0x0c to draw high score data?
	// we initialize these to 0xfe when the MCU is 'reset'

	if (!machine().side_effects_disabled())
		logerror("%s: bkungfu_blitter_r %04x\n", machine().describe_context(), offset);

	return m_blittercmdram[offset];
}

void m62_bkungfu_state::bkungfu_blitter_draw_text_inner(uint16_t blitterromptr, bool use_ram)
{
	const int tilemap_width = 64;
	const int widthmask = (tilemap_width * 2) - 1;
	//int tilemap_height = 32;

	if (!m_mcu_running)
		return;

	uint16_t data_address;
	const uint8_t *dataptr;

	uint8_t poslow_attr;
	uint8_t poshigh_attr;
	uint8_t col_attr;

	if (use_ram) // high score table is drawn from RAM
	{
		data_address = 0x100;
		dataptr = m_blittercmdram;
		// attribute 2 is always set to 0xe1, and the rest are shifted in this mode, why?
		poslow_attr = 3;
		poshigh_attr = 4;
		col_attr = 5;
	}
	else
	{
		data_address = m_blitterdatarom[blitterromptr] | (m_blitterdatarom[blitterromptr + 1] << 8);
		dataptr = m_blitterdatarom;
		poslow_attr = 2;
		poshigh_attr = 3;
		col_attr = 4;
	}

	uint8_t blitdat = dataptr[data_address++];
	while (blitdat != 0x00)
	{
		if (blitdat == 0x01)
		{
			// change color value during blit
			m_blittercmdram[col_attr] = dataptr[data_address++];
		}
		else if (blitdat == 0x02)
		{
			// change position params during blit
			m_blittercmdram[poslow_attr] = dataptr[data_address++];
			m_blittercmdram[poshigh_attr] = dataptr[data_address++];
		}
		else
		{
			uint16_t position = (m_blittercmdram[poshigh_attr] << 8) | m_blittercmdram[poslow_attr];

			bkungfu_blitter_tilemap_w((position) & 0xfff, blitdat);
			bkungfu_blitter_tilemap_w((position + 1) & 0xfff, m_blittercmdram[col_attr]);

			// move along to the next character, wrapping at the end of a line
			// otherwise Game Over text will sometimes get split across lines due to crossing the right edge
			int vposition = (position & ~widthmask);
			position += 2;
			position = vposition | (position & widthmask);

			m_blittercmdram[poslow_attr] = position & 0xff;
			m_blittercmdram[poshigh_attr] = (position & 0xff00) >> 8;
		}

		blitdat = dataptr[data_address++];
	}
}

void m62_bkungfu_state::bkungfu_blitter_draw_text()
{
	uint16_t blitterromptr = m_blittercmdram[0x001] * 2;
	bkungfu_blitter_draw_text_inner(blitterromptr, false);
}

void m62_bkungfu_state::bkungfu_blitter_clear_tilemap()
{
	if (!m_mcu_running)
		return;

	for (int position = 0; position < 0x1000; position += 2)
	{
		bkungfu_blitter_tilemap_w((position) & 0xfff, m_blittercmdram[0x002]);
		bkungfu_blitter_tilemap_w((position + 1) & 0xfff, m_blittercmdram[0x001]);
	}
}

uint8_t m62_bkungfu_state::bkungfu_blitter_decode_payload(uint16_t address) const
{
	uint8_t const value = m_blitterdecrypted[address];
	uint8_t const sum = uint8_t((address & 0xff) + (address >> 8));
	if (sum & 1)
		return uint8_t(0x60 - value);

	uint8_t const base = (value & 0x20) ? 0xa0 : 0x60;
	return uint8_t(base - value - ((value & 1) ? 0 : 2));
}

uint16_t m62_bkungfu_state::bkungfu_blitter_decode_payload_word(uint16_t address) const
{
	return bkungfu_blitter_decode_payload(address) | (bkungfu_blitter_decode_payload(address + 1) << 8);
}

void m62_bkungfu_state::bkungfu_blitter_draw_object(uint8_t id)
{
	// cmd 0x0f objects (0x80-0x90): decrypted table at 0x100 + 2*(id-0x80) points to a
	// 5-byte record [width, pos_lo, pos_hi, ptr_lo, ptr_hi]; the data stream at ptr is
	// '5f xx' attribute tokens + one byte per cell, terminated by 0x5e/0x60.
	//
	// Payload bytes have one more small address-dependent transform after the data-ROM
	// decryption.  The exact same transform is used for tile and attribute bytes; the
	// decoded attribute is already in M62 tile-RAM format.  This reproduces both title
	// objects and both flame frames without object-specific masks, cursors or palettes.
	if (id < 0x80 || id > 0x90)
		return;

	uint16_t const recaddr = m_blitterdecrypted[0x100 + 2 * (id - 0x80)] | (m_blitterdecrypted[0x100 + 2 * (id - 0x80) + 1] << 8);
	if (recaddr == 0 || recaddr >= 0x8000 - 5)
		return;

	uint8_t const width = m_blitterdecrypted[recaddr];
	uint16_t const pos = (m_blitterdecrypted[recaddr + 1] | (m_blitterdecrypted[recaddr + 2] << 8)) & 0xfff;
	uint16_t dataptr = m_blitterdecrypted[recaddr + 3] | (m_blitterdecrypted[recaddr + 4] << 8);
	if (width == 0 || width > 0x20 || dataptr >= 0x8000)
		return;

	// Collect decoded tile-RAM bytes.  Control bytes are part of the stream grammar
	// and are not payload-decoded.
	uint8_t attrs[512], tiles[512];
	int count = 0;
	uint8_t attr = 0;
	while (dataptr < 0x8000 && count < 512)
	{
		uint16_t const address = dataptr;
		uint8_t const b = m_blitterdecrypted[dataptr++];
		if (b == 0x5f)
		{
			if (dataptr >= 0x8000)
				break;
			attr = bkungfu_blitter_decode_payload(dataptr);
			dataptr++;
			continue;
		}
		if (b == 0x5e || b == 0x60)
			break;
		tiles[count] = bkungfu_blitter_decode_payload(address);
		attrs[count] = attr;
		count++;
	}

	int const height = count / width;
	for (int k = 0; k < count; k++)
	{
		int const col = k % width;
		int const y = k / width;
		uint16_t const cell = (pos + y * 0x80 + col * 2) & 0xfff;
		bkungfu_blitter_tilemap_w(cell, tiles[k]);
		bkungfu_blitter_tilemap_w(cell + 1, attrs[k]);
	}

	logerror("blitter: draw object %02x rec@%04x w=%d h=%d pos=%03x count=%d\n", id, recaddr, width, height, pos, count);
}

void m62_bkungfu_state::bkungfu_blitter_draw_4_tile_column_row(int column, int row, uint8_t tile, uint8_t attr)
{
	// rows below 6 are the HUD
	row += 6;

	int offset = (row * 256);
	offset += (column * 4);
	offset <<= 1;

	for (int i = 0; i < 8; i += 2)
	{
		m_bkungfu_tileram[offset + i] = tile;
		m_bkungfu_tileram[offset + i + 1] = attr;

		m_bg_tilemap->mark_tile_dirty((offset + i) >> 1);

	}
}

void m62_bkungfu_state::bkungfu_blitter_draw_4_tile_column(int column, int row)
{
	// The level table is consumed in pairs.  The even record decodes to 20 rows
	// and supplies ten upper rows to each of two adjacent destination strips.
	// The odd record decodes to 32 rows and supplies sixteen lower rows to each:
	//
	//   destination even = even[0..9]  + odd[0..15]
	//   destination odd  = even[10..19] + odd[16..31]
	//
	// Thus each record pair expands to two 4-tile-wide by 26-row playfield
	// strips.  The six HUD rows are not present in this level payload.
	int const destcolumn = column;
	bkungfu_blitter_draw_4_tile_column_row(destcolumn, row, 0x05, 0x19);

	int const source_entry = (column & ~1) + ((row >= 10) ? 1 : 0);
	int const source_row = (row < 10)
		? row + ((column & 1) ? 10 : 0)
		: row - 10 + ((column & 1) ? 16 : 0);

	// The Z80 advances its level value modulo eight, so gameplay order is
	// 1,2,...,7,0 rather than 0,1,...,7.  The MCU uses that value directly as
	// the master-table index.  Redraw values are the same indices plus eight.
	// Consequently table slot 0 is gameplay stage 8, not stage 1.
	uint16_t const table = 0x200 + ((m_leveldraw_number & 0x0f) << 1);
	uint16_t const block = m_blitterdecrypted[table] | (m_blitterdecrypted[table + 1] << 8);
	uint16_t const entry = block + source_entry * 2;
	if (block < 0x153d || entry >= 0x8000 - 1)
		return;

	uint16_t const record = bkungfu_blitter_decode_payload_word(entry);
	if (record < 0x1aed || record >= 0x8000 - 8)
		return;

	uint8_t tiles[4][64];
	uint8_t attrs[4][64];
	int count = -1;
	for (int stream = 0; stream < 4; stream++)
	{
		uint16_t dataptr = bkungfu_blitter_decode_payload_word(record + stream * 2);
		if (dataptr < 0x2349 || dataptr >= 0x8000)
			return;

		uint8_t attr = 0;
		int streamcount = 0;
		while (dataptr < 0x8000 && streamcount < 64)
		{
			uint8_t const value = bkungfu_blitter_decode_payload(dataptr++);
			if (value == 0x00)
				break;
			if (value == 0x01)
			{
				if (dataptr >= 0x8000)
					return;
				attr = bkungfu_blitter_decode_payload(dataptr++);
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
	int const offset = ((row + 6) * 256 + destcolumn * 4) * 2;
	for (int x = 0; x < 4; x++)
	{
		int const stream = band * 2 + x / 2;
		int const index = bandrow * 2 + x % 2;
		m_bkungfu_tileram[offset + x * 2] = tiles[stream][index];
		m_bkungfu_tileram[offset + x * 2 + 1] = attrs[stream][index];
		m_bg_tilemap->mark_tile_dirty((offset + x * 2) >> 1);
	}
}

TIMER_CALLBACK_MEMBER(m62_bkungfu_state::leveldraw_next)
{
	bkungfu_blitter_draw_4_tile_column(m_leveldraw_column, m_leveldraw_row);

	m_leveldraw_row++;

	if (m_leveldraw_row == 26)
	{
		m_leveldraw_row = 0;
		m_leveldraw_column++;
	}

	if (m_leveldraw_column != 0x38)
	{
		m_leveldraw_timer->adjust(attotime::from_usec(200));
	}
	else
	{
		m_blittercmdram[0x00] = 0xfe;
	}
}


void m62_bkungfu_state::bkungfu_blitter_tilemap_w(uint16_t offset, uint8_t data)
{
	// the tilemap needs to be 256 tiles wide for the backgrounds, which are copied in a single command
	// however the blitter commands seem to only have enough co-ordinates for the current 64 tile page
	// and the higher bits aren't communicated to the MCU, so assume they mirror across all pages for now
	//
	// It's also possible the tilemap is still 64 tiles wide, like kungfum and the MCU is loading in
	// backgrounds as needed, even if the command to draw the background is only sent at the start of
	// a level.  The draw-in time on the background might give clues to this.

	int xpart = offset & 0x7f;
	int ypart = offset & ~0x7f;

	for (int page = 0; page < 0x200; page += 0x80)
	{
		int realoffset = (ypart << 2) | xpart | page;

		m_bkungfu_tileram[realoffset] = data;
		m_bg_tilemap->mark_tile_dirty(realoffset >> 1);
	}
}

void m62_bkungfu_state::machine_start()
{
	m62_state::machine_start();

	save_item(NAME(m_blittercmdram));
	save_item(NAME(m_mcu_running));

	save_item(NAME(m_hud_timer));
	save_item(NAME(m_hud_p1score));
	save_item(NAME(m_hud_topscore));
	save_item(NAME(m_hud_p2score));
	save_item(NAME(m_hud_lives));
	save_item(NAME(m_hud_player_energy));
	save_item(NAME(m_hud_boss_energy));
	save_item(NAME(m_hud_floorcount));
	save_item(NAME(m_hud_floorcount_state));
	save_item(NAME(m_leveldraw_row));
	save_item(NAME(m_leveldraw_column));
	save_item(NAME(m_leveldraw_number));

	m_leveldraw_timer = timer_alloc(FUNC(m62_bkungfu_state::leveldraw_next), this);

	// decrypt the blitter data ROM (0x153d-0x7fff): key index s = (lo+hi)&0xff of the
	// byte address; even s: plain = cipher ^ K[s], odd s: plain = (K[s] - cipher) & 0xff
	static const uint8_t blitter_key[256] = {
	0xae, 0xf3, 0x5c, 0x5d, 0xaa, 0xf7, 0x58, 0x59, 0xa6, 0xfb, 0x54, 0x55, 0xa2, 0xff, 0x50, 0x51,
	0xbe, 0xe3, 0x4c, 0x4d, 0xba, 0xe7, 0x48, 0x49, 0xb6, 0xeb, 0x44, 0x45, 0xb2, 0xef, 0x40, 0x41,
	0x8e, 0x13, 0x7c, 0x3d, 0x8a, 0x17, 0x78, 0x39, 0x86, 0x1b, 0x74, 0x35, 0x82, 0x1f, 0x70, 0x31,
	0x9e, 0x03, 0x6c, 0x2d, 0x9a, 0x07, 0x68, 0x29, 0x96, 0x0b, 0x64, 0x25, 0x92, 0x0f, 0x60, 0x21,
	0xee, 0x33, 0x1c, 0x1d, 0xea, 0x37, 0x18, 0x19, 0xe6, 0x3b, 0x14, 0x15, 0xe2, 0x3f, 0x10, 0x11,
	0xfe, 0x23, 0x0c, 0x0d, 0xfa, 0x27, 0x08, 0x09, 0xf6, 0x2b, 0x04, 0x05, 0xf2, 0x2f, 0x00, 0x01,
	0xce, 0x53, 0x3c, 0xfd, 0xca, 0x57, 0x38, 0xf9, 0xc6, 0x5b, 0x34, 0xf5, 0xc2, 0x5f, 0x30, 0xf1,
	0xde, 0x43, 0x2c, 0xed, 0xda, 0x47, 0x28, 0xe9, 0xd6, 0x4b, 0x24, 0xe5, 0xd2, 0x4f, 0x20, 0xe1,
	0x2e, 0x73, 0xdc, 0xdd, 0x2a, 0x77, 0xd8, 0xd9, 0x26, 0x7b, 0xd4, 0xd5, 0x22, 0x7f, 0xd0, 0xd1,
	0x3e, 0x63, 0xcc, 0xcd, 0x3a, 0x67, 0xc8, 0xc9, 0x36, 0x6b, 0xc4, 0xc5, 0x32, 0x6f, 0xc0, 0xc1,
	0x0e, 0x93, 0xfc, 0xbd, 0x0a, 0x97, 0xf8, 0xb9, 0x06, 0x9b, 0xf4, 0xb5, 0x02, 0x9f, 0xf0, 0xb1,
	0x1e, 0x83, 0xec, 0xad, 0x1a, 0x87, 0xe8, 0xa9, 0x16, 0x8b, 0xe4, 0xa5, 0x12, 0x8f, 0xe0, 0xa1,
	0x6e, 0xb3, 0x9c, 0x9d, 0x6a, 0xb7, 0x98, 0x99, 0x66, 0xbb, 0x94, 0x95, 0x62, 0xbf, 0x90, 0x91,
	0x7e, 0xa3, 0x8c, 0x8d, 0x7a, 0xa7, 0x88, 0x89, 0x76, 0xab, 0x84, 0x85, 0x72, 0xaf, 0x80, 0x81,
	0x4e, 0xd3, 0xbc, 0x7d, 0x4a, 0xd7, 0xb8, 0x79, 0x46, 0xdb, 0xb4, 0x75, 0x42, 0xdf, 0xb0, 0x71,
	0x5e, 0xc3, 0xac, 0x6d, 0x5a, 0xc7, 0xa8, 0x69, 0x56, 0xcb, 0xa4, 0x65, 0x52, 0xcf, 0xa0, 0x61,
	};
	m_blitterdecrypted = std::make_unique<uint8_t []>(0x8000);
	for (int a = 0; a < 0x8000; a++)
	{
		uint8_t const c = m_blitterdatarom[a];
		if (a < 0x153d)
			m_blitterdecrypted[a] = c;
		else
		{
			uint8_t const s = ((a & 0xff) + (a >> 8)) & 0xff;
			m_blitterdecrypted[a] = (s & 1) ? uint8_t(blitter_key[s] - c) : uint8_t(c ^ blitter_key[s]);
		}
	}
}

void m62_bkungfu_state::machine_reset()
{
	m62_state::machine_reset();

	for (int i = 0; i < 0x800; i++)
		m_blittercmdram[i] = 0x00;

	m_mcu_running = 0;

	m_hud_timer = 0;
	m_hud_p1score = 0;
	m_hud_topscore = 0;
	m_hud_p2score = 0;
	m_hud_lives = 0;
	m_hud_player_energy = 0;
	m_hud_boss_energy = 0;
	m_hud_floorcount = 0;
	m_hud_floorcount_state = 0;
	m_leveldraw_row = 0;
	m_leveldraw_column = 0;
	m_leveldraw_number = 0;

	m_leveldraw_timer->adjust(attotime::never);
}


void m62_bkungfu_state::bkungfu_blitter_set_floor_state(int which, int state)
{
	const int y = 3;
	const int x = 0x20 + which * 2;

	int position = (y * 0x40) + x; // 0x40 tiles per line
	position <<= 1; // 2 bytes per entry in tilemap
	bkungfu_blitter_tilemap_w(position, state ? 0xd5 : 0xd6);
}


void m62_bkungfu_state::bkungfu_blitter_set_number(int x, int y, uint8_t num)
{
	int position = (y * 0x40) + x; // 0x40 tiles per line
	position <<= 1; // 2 bytes per entry in tilemap
	bkungfu_blitter_tilemap_w(position, (num & 0xf) + 0x30);
}

void m62_bkungfu_state::bkungfu_blitter_draw_credits_continue()
{
	// draws a 2 digit number from a byte to a given location with given attribute
	if (!m_mcu_running)
		return;

	uint16_t position = (m_blittercmdram[0x003] << 8) | m_blittercmdram[0x002];
	uint8_t attr = m_blittercmdram[0x004];

	int value = (m_blittercmdram[0x001] & 0xf0) >> 4;
	value += 0x30;

	bkungfu_blitter_tilemap_w((position) & 0xfff, value);
	bkungfu_blitter_tilemap_w((position + 1) & 0xfff, attr);

	position += 2;

	value = m_blittercmdram[0x001] & 0x0f;
	value += 0x30;

	bkungfu_blitter_tilemap_w((position) & 0xfff, value);
	bkungfu_blitter_tilemap_w((position + 1) & 0xfff, attr);
}

void m62_bkungfu_state::bkungfu_blitter_set_player_energy(int x, int y, uint8_t num, bool is_boss)
{
	if (num > 8)
		return;

	constexpr uint8_t energy_table_player[9] = { 0xc2, 0xcb, 0xca, 0xc9, 0xc8, 0xc7, 0xc6, 0xc5, 0xc4 };
	constexpr uint8_t energy_table_boss[9] = { 0xc2, 0xd3, 0xd2, 0xd1, 0xd0, 0xcf, 0xce, 0xcd, 0xcc };

	int position = (y * 0x40) + x; // 0x40 tiles per line
	position <<= 1; // 2 bytes per entry in tilemap

	if (is_boss)
		bkungfu_blitter_tilemap_w(position, energy_table_boss[num]);
	else
		bkungfu_blitter_tilemap_w(position, energy_table_player[num]);
}

void m62_bkungfu_state::bkungfu_blitter_draw_lifebar(int xbase, int ybase, uint8_t energy, bool is_boss)
{
	// the energy bar values range from 0x00 (empty) to 0x40 (full)
	// player tiles 0xc4 - full bar, 0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb (1 line left)
	// 0xc2 empty
	//
	// player bar is at 0x12e in tile ram and consists of 8 segments
	// state 0x40 is all full, so 0xc4,0xc4,0xc4,0xc4,0xc4,0xc4,0xc4,0xc4

	const int num8segments = (energy & 0x78) >> 3;
	int segment = 0;
	while (segment < num8segments)
	{
		// draw the full bar parts
		bkungfu_blitter_set_player_energy(xbase + segment, ybase, 8, is_boss);
		segment++;
	}
	if (segment != 8)
	{
		// draw the partial bar parts
		bkungfu_blitter_set_player_energy(xbase + segment, ybase, energy & 0x7, is_boss);
		segment++;
	}
	while (segment < 8)
	{
		// draw the empty bar parts
		bkungfu_blitter_set_player_energy(xbase + segment, ybase, 0, is_boss);
		segment++;
	}
}

void m62_bkungfu_state::redraw_hud()
{
	// draw the static part of the layout
	bkungfu_blitter_draw_text_inner(0x140, false);

	// update the dynamic parts of the layout
	bkungfu_blitter_set_number(0x25, 0x05, ((m_hud_timer >> 8) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x26, 0x05, ((m_hud_timer >> 8) & 0x0f));
	bkungfu_blitter_set_number(0x27, 0x05, ((m_hud_timer >> 0) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x28, 0x05, ((m_hud_timer >> 0) & 0x0f));

	bkungfu_blitter_set_number(0x2d, 0x05, ((m_hud_lives >> 0) & 0x0f));

	bkungfu_blitter_set_number(0x14, 0x00, ((m_hud_p1score >> 16) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x15, 0x00, ((m_hud_p1score >> 16) & 0x0f));
	bkungfu_blitter_set_number(0x16, 0x00, ((m_hud_p1score >> 8) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x17, 0x00, ((m_hud_p1score >> 8) & 0x0f));
	bkungfu_blitter_set_number(0x18, 0x00, ((m_hud_p1score >> 0) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x19, 0x00, ((m_hud_p1score >> 0) & 0x0f));

	bkungfu_blitter_set_number(0x1f, 0x00, ((m_hud_topscore >> 16) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x20, 0x00, ((m_hud_topscore >> 16) & 0x0f));
	bkungfu_blitter_set_number(0x21, 0x00, ((m_hud_topscore >> 8) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x22, 0x00, ((m_hud_topscore >> 8) & 0x0f));
	bkungfu_blitter_set_number(0x23, 0x00, ((m_hud_topscore >> 0) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x24, 0x00, ((m_hud_topscore >> 0) & 0x0f));

	bkungfu_blitter_set_number(0x29, 0x00, ((m_hud_p2score >> 16) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x2a, 0x00, ((m_hud_p2score >> 16) & 0x0f));
	bkungfu_blitter_set_number(0x2b, 0x00, ((m_hud_p2score >> 8) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x2c, 0x00, ((m_hud_p2score >> 8) & 0x0f));
	bkungfu_blitter_set_number(0x2d, 0x00, ((m_hud_p2score >> 0) & 0xf0) >> 4);
	bkungfu_blitter_set_number(0x2e, 0x00, ((m_hud_p2score >> 0) & 0x0f));

	if (m_hud_player_energy <= 0x40)
	{
		bkungfu_blitter_draw_lifebar(0x17, 0x2, m_hud_player_energy, false);
	}

	if (m_hud_boss_energy <= 0x40)
	{
		bkungfu_blitter_draw_lifebar(0x17, 0x4, m_hud_boss_energy, true);
	}

	for (int i = 0; i < 8; i++)
	{
		int numcoloured = m_hud_floorcount;

		// uses this to flash the current floor counter dot
		if (m_hud_floorcount_state == 0x02)
			numcoloured--;

		if (i <= numcoloured)
			bkungfu_blitter_set_floor_state(i, 1);
		else
			bkungfu_blitter_set_floor_state(i, 0);

	}
}

void m62_bkungfu_state::bkungfu_blitter_w(offs_t offset, uint8_t data)
{
	int pc = m_maincpu->pc();

	m_blittercmdram[offset] = data;

	if (offset == 0x00)
	{
		if (data == 0x14)
		{
			logerror("%s: Command %02x: blitter: draw text from ROM\n", machine().describe_context(), data);
			bkungfu_blitter_draw_text();
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x0d)
		{
			// this is used on the ending, and to draw the headers for the high score table
			// should it differ from the above somehow, or is it just a mirrored command?
			logerror("%s: Command %02x: blitter: draw text from ROM (alt)\n", machine().describe_context(), data);
			bkungfu_blitter_draw_text();
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x0c)
		{
			// why is it checking 4 addresses for 0xfe before writing the command?
			//':maincpu' (7DDA): bkungfu_blitter_r 0102
			//':maincpu' (7DDF): bkungfu_blitter_r 0106
			//':maincpu' (7DDA): bkungfu_blitter_r 0118
			//':maincpu' (7DDF): bkungfu_blitter_r 011c
			//':maincpu' (7DCE): Command 0c: blitter: draw highscores

			// these are written before the call (always 00 e1?)
			uint8_t param1 = m_blittercmdram[0x001];
			uint8_t param2 = m_blittercmdram[0x002];
			uint8_t param3 = m_blittercmdram[0x003];
			uint8_t param4 = m_blittercmdram[0x004];
			uint8_t param5 = m_blittercmdram[0x005];

			logerror("%s: Command %02x: blitter: draw highscores %02x %02x %02x %02x %02x\n", machine().describe_context(), data, param1, param2, param3, param4, param5);
			bkungfu_blitter_draw_text_inner(-1, true);

			// override the RAM data with 0xfe (task done flag) so this works for the next call
			// (seems logical based on checks above) but works without it, and breaks if you do this?)
			//  m_blittercmdram[0x102] = 0xfe;
			//  m_blittercmdram[0x106] = 0xfe;
			//  m_blittercmdram[0x118] = 0xfe;
			//  m_blittercmdram[0x11c] = 0xfe;
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x10)
		{
			// displays the number of coins after 'CREDIT' and the 'CONTINUE' counter
			// writes position data(?) to 0x02 / 0x03
			// write number of credits to param 0x01 and 0x1d (attribute?) to 0x04
			uint8_t param = m_blittercmdram[0x001];
			logerror("%s: Command %02x: blitter: draw number of coins %02x\n", machine().describe_context(), data, param);
			bkungfu_blitter_draw_credits_continue();
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x08) // clear layer to fixed value
		{
			logerror("%s: Command %02x: blitter: clear layer\n", machine().describe_context(), data);
			bkungfu_blitter_clear_tilemap();
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x02)
		{
			// triggered just afer command 0x01 below
			// this might trigger the actual drawing if the previous commands were just the set-up for it?

			uint8_t param1 = m_blittercmdram[0x001];
			uint8_t param2 = m_blittercmdram[0x002];
			logerror("%s: Command %02x: blitter: start of level cmd 2 (do draw?) %02x %02x\n", machine().describe_context(), data, param1, param2);

			// note this isn't instant, you can see the draw-in on PCB footage so we're using a timer
			// however it should complete before the character starts walking, it doesn't, is this the wrong trigger?
			// or should this command cycle steal the maincpu instead? (there doesn't seem to be any wait for it to complete)
			m_leveldraw_row = 0;
			m_leveldraw_column = 0;
			m_leveldraw_number = param1;
			m_leveldraw_timer->adjust(attotime::from_usec(200));
		}
		else if (data == 0x01)
		{
			// see notes at top of driver

			// triggered just after command 0x0a below, and before command 0x02 above
			// it happens twice at the start of each stage, once before any level animations are complete
			// the param is different each time +8 the 2nd time, as to point to the 'with animations complete' state of the tilemap

			uint16_t blitterromptr = m_blittercmdram[0x001];
			uint16_t data_address = m_blitterdatarom[0x200 + (blitterromptr << 1)] | (m_blitterdatarom[0x200 + (blitterromptr << 1) + 1] << 8);
			//popmessage("%s: Command %02x: blitter: draw level from ROM initialize - param %02x source address is %04x", machine().describe_context(), data, blitterromptr, data_address);
			logerror("%s: Command %02x: blitter: draw level from ROM - param %02x source address is %04x\n", machine().describe_context(), data, blitterromptr, data_address);
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x0a)
		{
			// this happens BEFORE the level drawing commands, at the start of a stage
			// the level number is in the params, maybe it's used to draw the HUD in a default state?
			// (unlikely, it gets called without updating the other elements after you die etc.)
			uint16_t levelnum = m_blittercmdram[0x001];
			logerror("%s: Command %02x: blitter: pre-draw level (draw HUD?) %02x\n", machine().describe_context(), data, levelnum);
			redraw_hud();
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x05)
		{
			// used at the start of stages 4,5,6,7,8 when drawing animated stage elements
			//
			// these are simple animations such as a door or trapdoor closing
			// it's called after command 0xf in those cases, so see details in that command
			//
			// no additional params are sent after calling 0xf and before calling this?
			// which could suggest command 0xf is used to draw instead, but then why this call after it?
			logerror("%s: Command %02x: blitter: after level animation command on level 3+\n", machine().describe_context(), data);
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0x0f)
		{
			// used in the attract mode when drawing the background of the title screen
			// and before animated level elements at the start of later stages

			// writes to param offsets 1/2 (same value for each) before this command
			// uses values of 85, 84, 83, 86 (then sits on 90 at the end of the sequence) on the title screen
			//
			// values 8b, 8c, 8d, 8e, 8f are used for the door on the 4th level (5 frames of animation)
			// values 80, 81 are used for the trapdoor on the 5th level (2 frames of animation)
			// values 87, 88 are used for the trapdoor on the 6th level (2 frames of animation)
			// values 80, 81 are used for the trapdoor on the 7th level (2 frames of animation) (same as 5th level)
			// values 89, 8a are used for the trapdoor on the 8th level (2 frames of animation)
			//
			// so object values are between 0x80 and 0x90, but 0x82 seems unused
			// there doesn't appear to be any unencrypted data for these in the data ROM, so the object
			// definitions are either coming from internal MCU ROM or the encrypted area

			logerror("%s: Command %02x: blitter: draw title animation element (flames / level animations) %02x %02x\n", machine().describe_context(), data, m_blittercmdram[0x001], m_blittercmdram[0x002]);
			bkungfu_blitter_draw_object(m_blittercmdram[0x001]);
			m_blittercmdram[0x00] = 0xfe;
		}
		else if (data == 0xfe)
		{
			// probably tells the MCU to start running
			m_mcu_running = 1;
			logerror("%s: Command %02x: blitter: start up\n", machine().describe_context(), data);

			// set the command response addresses to ready / done
			// (the high score table commands check their status before executing anything)

			// HUD Status
			m_blittercmdram[0x010] = 0xfe;
			m_blittercmdram[0x014] = 0xfe;
			m_blittercmdram[0x018] = 0xfe;
			m_blittercmdram[0x01c] = 0xfe;
			m_blittercmdram[0x020] = 0xfe;
			m_blittercmdram[0x024] = 0xfe;
			m_blittercmdram[0x028] = 0xfe;
			m_blittercmdram[0x02c] = 0xfe;

			// High Score Table
			m_blittercmdram[0x102] = 0xfe;
			m_blittercmdram[0x106] = 0xfe;
			m_blittercmdram[0x118] = 0xfe;
			m_blittercmdram[0x11c] = 0xfe;

			// trigger
			m_blittercmdram[0x00] = 0xfe;
		}
		else
		{
			logerror("%s: Command %02x: blitter: unknown\n", machine().describe_context(), data);
		}
	}
	// all these 'slots' are initialized when you start a game
	// there seem to be 3 param bytes and a trigger address for each
	else if ((offset >= 0x10) && (offset < 0x30))
	{

		int select = offset & 0x3c;
		int part = offset & 0x03;

		if (part == 0x00)
		{
			if (!m_mcu_running)
				return;

			// all these commands draw the HUD in various states
			//
			// a pointer to this basic HUD layout is at 0x140, although it could be a leftover
			// as the commands below all require elements of it to be different rather than a
			// static layout
			//
			// the parts needed for these elements (eg. partial life bars) aren't in the MCU data
			// ROM anywhere apart from a static version, so probably come from internal ROM or
			// encrypted area
			//
			// we draw the static HUD in command 0xa instead, as these likely just update parts of it

			switch (select)
			{
			case 0x10:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (player 1 score draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);

				m_hud_p1score =  m_blittercmdram[offset + 1] << 16;
				m_hud_p1score |= m_blittercmdram[offset + 2] << 8;
				m_hud_p1score |= m_blittercmdram[offset + 3] << 0;
				redraw_hud();
				break;
			}
			case 0x14:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (player 2 score draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);

				m_hud_p2score =  m_blittercmdram[offset + 1] << 16;
				m_hud_p2score |= m_blittercmdram[offset + 2] << 8;
				m_hud_p2score |= m_blittercmdram[offset + 3] << 0;
				redraw_hud();
				break;
			}
			case 0x18:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (top score draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);
				m_hud_topscore =  m_blittercmdram[offset + 1] << 16;
				m_hud_topscore |= m_blittercmdram[offset + 2] << 8;
				m_hud_topscore |= m_blittercmdram[offset + 3] << 0;
				redraw_hud();
				break;
			}
			case 0x1c:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (timer draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);
				m_hud_timer =  m_blittercmdram[offset + 1] << 8;
				m_hud_timer |= m_blittercmdram[offset + 2] << 0;
				redraw_hud();
				break;
			}
			case 0x20:
			{
				// this is triggered with 0x01 and 0x02, the counter display is meant to flash between 2 states like in the original
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (floor counter draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);
				m_hud_floorcount = m_blittercmdram[offset + 1];
				m_hud_floorcount_state = data;
				redraw_hud();
				break;
			}
			case 0x24:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (lives counter draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);
				m_hud_lives = m_blittercmdram[offset + 1];
				redraw_hud();
				break;
			}
			case 0x28:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (player energy draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);
				m_hud_player_energy = m_blittercmdram[offset + 1];
				redraw_hud();
				break;
			}
			case 0x2c:
			{
				logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (boss energy draw %02x %02x %02x)\n", machine().describe_context(), offset, data, m_blittercmdram[offset+1], m_blittercmdram[offset+2], m_blittercmdram[offset+3]);
				m_hud_boss_energy = m_blittercmdram[offset + 1];
				redraw_hud();
				break;
			}
			}

			m_blittercmdram[offset] = 0xfe; // flag 'done' on the trigger address in shared RAM
		}

	}
	// used on the high score table, is this just data for other commands?
	else if ((offset >= 0x100) && (offset <= 0x12c))
	{
		// text format data used for drawing high score table?
		// used by command 0x0c at offest 0x00
		logerror("%s: bkungfu_blitter_w offset: %04x data: %02x (high score table related)\n", machine().describe_context(), offset, data);
	}
	else
	{
		if ((pc != 0x122c) && (pc != 0x0bd5))
		{
			// don't log initial RAM test 0x122c
			// or the scroll MSB it writes during gameplay (0x0bd5)
			logerror("%s: bkungfu_blitter_w offset: %04x data: %02x\n", machine().describe_context(), offset, data);
		}
	}
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void m62_bkungfu_state::mem_map(address_map& map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc0ff).ram().share("spriteram");
	map(0xc100, 0xc1ff).ram();

	map(0xc800, 0xcfff).rw(FUNC(m62_bkungfu_state::bkungfu_blitter_r), FUNC(m62_bkungfu_state::bkungfu_blitter_w));
	map(0xe000, 0xefff).ram();
}

void m62_bkungfu_state::io_map(address_map &map)
{
	kungfum_io_map(map);
	map(0x80, 0x80).w(FUNC(m62_bkungfu_state::m62_hscroll_low_w));
	map(0x81, 0x81).w(FUNC(m62_bkungfu_state::m62_hscroll_high_w));
	map(0x83, 0x83).w(FUNC(m62_bkungfu_state::kidniki_background_bank_w));
	//map(0x84, 0x84).nopw();
}



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void m62_bkungfu_state::bkungfu(machine_config& config)
{
	kungfum(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &m62_bkungfu_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &m62_bkungfu_state::io_map);

	MCFG_VIDEO_START_OVERRIDE(m62_bkungfu_state,bkungfu)
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( bkungfu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "km-a.4e", 0x00000, 0x4000, CRC(083632aa) SHA1(0a52c6162b2fb55057735a54c59f7cb88d870593) )
	ROM_LOAD( "km-a.4d", 0x04000, 0x4000, CRC(08b14684) SHA1(8d60abe5f06e1b3ce465ec740df3f4ee8e9398bc) )
	ROM_LOAD( "km-z.7j", 0x08000, 0x4000, CRC(2bd2aa83) SHA1(422ef2a64f040a0974311ff692726c6f3a8f8b13) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "mcu",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x10000, "blitterdat", ROMREGION_ERASEFF )
	ROM_LOAD( "km-z.4h", 0x0000, 0x8000, CRC(252bb4a9) SHA1(2a69ee113950ea58895b42102bbb5263865ace9d) )

	ROM_REGION( 0x10000, "irem_audio:iremsound", 0 )
	ROM_LOAD( "km-a.3a", 0x4000, 0x4000, CRC(bb709dd6) SHA1(aa491ec2d64b096927546b4362fa41c4784659c9) )
	ROM_LOAD( "km-a.3d", 0x8000, 0x4000, CRC(ef8551cb) SHA1(2a26feeae8ea7ddc5d899592bc4c17b40571fe8f) )
	ROM_LOAD( "km-a.3f", 0xc000, 0x4000, CRC(bec93bdc) SHA1(e287d3e689e18b192d686f313c63b6ed4e10a83f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "km-z.3d", 0x00000, 0x8000, CRC(a8007429) SHA1(5e315ba41bbb2248cf49b1fd0a1601c08bf891b4) )
	ROM_LOAD( "km-z.3c", 0x08000, 0x8000, CRC(a99be837) SHA1(48b720462b359ef3551d50b8a41d2e3a2e146a60) )
	ROM_LOAD( "km-z.3a", 0x10000, 0x8000, CRC(0bb7fda0) SHA1(aaab12b5e5402f3bbd1a2700ecc65dc588f0e590) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "km-b.4k", 0x00000, 0x4000, CRC(7e8bec97) SHA1(6ca5939bd64df63124de997b53c9ac9975450ab5) )
	ROM_LOAD( "km-b.4f", 0x04000, 0x4000, CRC(3a75305b) SHA1(f62e7a293647be8aabbdd0b366459f634de593c6) )
	ROM_LOAD( "km-b.4l", 0x08000, 0x4000, CRC(fe746127) SHA1(53f446a28466e0a749c27aa2eeb7d7bad4bd8d9b) )
	ROM_LOAD( "km-b.4h", 0x0c000, 0x4000, CRC(f322c5dd) SHA1(03cb9658f31853f1ba41d8bf7e8cbc87353cc432) )
	ROM_LOAD( "km-b.3n", 0x10000, 0x4000, CRC(b88a4d16) SHA1(2ab45a4bf44b5827def8166b30faa08514e7a814) )
	ROM_LOAD( "km-b.4n", 0x14000, 0x4000, CRC(f92be992) SHA1(dd3ddc1ba76ceba71435a63c43f1be24a3272011) )
	ROM_LOAD( "km-b.4m", 0x18000, 0x4000, CRC(53623913) SHA1(efb3de824df15b95e5eb91b5907ae2232501e3e3) )
	ROM_LOAD( "km-b.3m", 0x1c000, 0x4000, CRC(1d1ec6f2) SHA1(fcdaf34529166701aad87d013176f4709cb5d540) )
	ROM_LOAD( "km-b.4c", 0x20000, 0x4000, CRC(31cb5b98) SHA1(54bb88d668c59ec5248098a053a99132a121df74) )
	ROM_LOAD( "km-b.4e", 0x24000, 0x4000, CRC(942e60fc) SHA1(d14553854b8300e80d7556b3be47544c537daac3) )
	ROM_LOAD( "km-b.4d", 0x28000, 0x4000, CRC(90a03502) SHA1(bf4efc5e170f8ae479411eef98d8c38bb32d2648) )
	ROM_LOAD( "km-b.4a", 0x2c000, 0x4000, CRC(018509c2) SHA1(844f3d79a4f358ccce1023deb76c052914570aed) )

	ROM_REGION( 0x300, "spr_color_proms", 0 )
	ROM_LOAD( "km-b.1m", 0x0000, 0x0100, CRC(73638418) SHA1(ad3f9cf08d334e76294bd796b7f8849014f05b8e) )
	ROM_LOAD( "km-b.1n", 0x0100, 0x0100, CRC(7967dfdf) SHA1(95f5aac75ce902287c0d0ada6ee9e1e1bd9d87c0) )
	ROM_LOAD( "km-b.1l", 0x0200, 0x0100, CRC(4f44ef5c) SHA1(094e6ab24a5663208fc9d54203ed200c9e4a9fc3) )

	ROM_REGION( 0x300, "chr_color_proms", 0 )
	ROM_LOAD( "km-z-1j", 0x0000, 0x0100, CRC(1da0ad7f) SHA1(3b3ba444efa8f5481c64b3c9ef1c0ab6561c9f16) )
	ROM_LOAD( "km-z-1h", 0x0100, 0x0100, CRC(23a217c0) SHA1(723738cd8875c5bb449be2da75ef0b04cca8dd55) )
	ROM_LOAD( "km-z-1f", 0x0200, 0x0100, CRC(ec9df4f2) SHA1(702f7d536a5f79987342521c66d703153a888010) )

	ROM_REGION( 0x20, "spr_height_prom", 0 )
	ROM_LOAD( "km-b.5p", 0x0000, 0x0020, CRC(33409e90) SHA1(b84f7df9c27aa18255099b0473c6088c6fd7adfa) )

	ROM_REGION( 0x100, "timing", 0 )
	ROM_LOAD( "km-b.6f", 0x0000, 0x0100, CRC(82c20d12) SHA1(268903f7d9be58a70d030b02bf31a2d6b5b6e249) )
ROM_END

} // anonymous namespace


/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  MACHINE  INPUT     CLASS              INIT        ROT     COMPANY  FULLNAME                          FLAGS
GAME( 1987, bkungfu, 0,      bkungfu, kungfum,  m62_bkungfu_state, empty_init, ROT0,   "Irem",  "Beyond Kung-Fu (location test)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
