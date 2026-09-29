// license:BSD-3-Clause
// copyright-holders:David Haywood, Andrea Bogazzi

/* MCU simulation code for IREM's Beyond Kung Fu
 
   The MCU is fully in charge of the tilemap layer, copying text strings
   and background data from a partially encrypted external ROM

   The exact MCU type is not confirmed
*/

#include "emu.h"
#include "m62_bkungfu_mcu.h"

/*

TODO:
- determine the purpose of the optional fifth/sixth level composition pointers
- background draw timing isn't 100% correct, although this has no impact on overall game timing
- does the hardware really have twice the VRAM as the other boards?
- test mode doesn't work (there are strings for it in the MCU data ROM, is the MCU involved, or is it incomplete)

*/

DEFINE_DEVICE_TYPE(BKUNG_MCU, bkungfu_mcu_device, "bkung_mcu", "Irem Beyond Kung-Fu MCU")

bkungfu_mcu_device::bkungfu_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BKUNG_MCU, tag, owner, clock)
	, m_tilemap_ram_w(*this)
	, m_mailbox_out_w(*this)
	, m_data_rom(*this, "blitterdat")
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

uint8_t bkungfu_mcu_device::read_data(uint16_t address) const
{
	return m_data_rom[address];
}

void bkungfu_mcu_device::vram_page_w(offs_t offset, uint8_t data)
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
		m_tilemap_ram_w(realoffset, data);
	}
}


uint8_t bkungfu_mcu_device::decrypt_data(uint16_t address) const
{
    if (address >= 0x8000)
        return 0xff;

    uint8_t const cipher = m_data_rom[address];
    uint8_t const index = uint8_t((address & 0xff) + (address >> 8));

    if (index & 1)
        return uint8_t(
            (index ^ ((index & 2) ? 0x7e : 0x12)) - 0x20 - cipher);

    return uint8_t(cipher ^ index ^ ((index & 2) ? 0x5e : 0xae));
}

void bkungfu_mcu_device::write_number(int x, int y, uint8_t number)
{
	vram_page_w(((y * 0x40 + x) << 1) & 0x0fff, (number & 0x0f) + 0x30);
}

void bkungfu_mcu_device::write_floor_dot(int which, bool lit)
{
	vram_page_w(((3 * 0x40 + 0x20 + which * 2) << 1) & 0x0fff, lit ? 0xd5 : 0xd6);
}

void bkungfu_mcu_device::write_lifebar(int xbase, int ybase, uint8_t energy, bool boss)
{
	int const full_segments = (energy & 0x78) >> 3;
	for (int segment = 0; segment < 8; segment++)
	{
		uint8_t const part = segment < full_segments ? 8 : segment == full_segments ? energy & 7 : 0;
		uint8_t const tile = part ? uint8_t((boss ? 0xcc : 0xc4) + 8 - part) : 0xc2;
		vram_page_w(((ybase * 0x40 + xbase + segment) << 1) & 0x0fff, tile);
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
	if (!m_running)
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
			vram_page_w(position & 0x0fff, value);
			vram_page_w((position + 1) & 0x0fff, m_mailbox[attribute]);
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
	vram_page_w(position & 0x0fff, (m_mailbox[1] >> 4) + 0x30);
	vram_page_w((position + 1) & 0x0fff, attribute);
	vram_page_w((position + 2) & 0x0fff, (m_mailbox[1] & 0x0f) + 0x30);
	vram_page_w((position + 3) & 0x0fff, attribute);
}

void bkungfu_mcu_device::clear_tilemap()
{
	if (!m_running)
		return;

	for (uint16_t position = 0; position < 0x1000; position += 2)
	{
		vram_page_w(position, m_mailbox[2]);
		vram_page_w(position + 1, m_mailbox[1]);
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
	if (!machine().side_effects_disabled())
		logerror("%s: mailbox_r %04x\n", machine().describe_context(), offset);

	return m_mailbox[offset];
}

void bkungfu_mcu_device::mailbox_from_main_w(offs_t offset, uint8_t data)
{
	if (offset < 0x800)
		mailbox_w(offset, data);

	if (offset == 0x00)
		command_w(data);
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
	if (id < 0x80 || id > 0x90)
		return;

	uint16_t const recaddr = read_data(0x100 + 2 * (id - 0x80)) | (uint16_t(read_data(0x100 + 2 * (id - 0x80) + 1)) << 8);
	if (recaddr == 0 || recaddr >= 0x8000 - 5)
		return;

	uint8_t const width = read_data(recaddr);
	uint16_t const pos = (read_data(recaddr + 1) | (uint16_t(read_data(recaddr + 2)) << 8)) & 0xfff;
	uint16_t dataptr = read_data(recaddr + 3) | (uint16_t(read_data(recaddr + 4)) << 8);
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
		vram_page_w(destination, tiles[cell]);
		vram_page_w(destination + 1, attrs[cell]);
	}
}

void bkungfu_mcu_device::draw_level_column_row(int column, int row, uint8_t tile, uint8_t attr)
{
	int const offset = ((row + 6) * 256 + column * 4) * 2;
	for (int x = 0; x < 4; x++)
	{
		m_tilemap_ram_w(offset + x * 2, tile);
		m_tilemap_ram_w(offset + x * 2 + 1, attr);
	}
}

void bkungfu_mcu_device::draw_level_strip(int column, int row)
{
	draw_level_column_row(column, row, 0x05, 0x19);
	int const source_entry = (column & ~1) + ((row >= 10) ? 1 : 0);
	int const source_row = (row < 10)
		? row + ((column & 1) ? 10 : 0)
		: row - 10 + ((column & 1) ? 16 : 0);
	uint16_t const table = 0x200 + ((m_leveldraw_number & 0x0f) << 1);
	uint16_t const block = read_data(table) | (uint16_t(read_data(table + 1)) << 8);
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
		m_tilemap_ram_w(offset + x * 2, tiles[stream][index]);
		m_tilemap_ram_w(offset + x * 2 + 1, attrs[stream][index]);
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
		vram_page_w(position & 0x0fff, value);
		vram_page_w((position + 1) & 0x0fff, attribute);
		position = (position & ~0x007f) | ((position + 2) & 0x007f);
	}

	m_initialized = true;
	for (uint8_t slot = 0x10; slot <= 0x2c; slot += 4)
		if (m_valid & (1U << ((slot - 0x10) >> 2)))
			update_slot(slot);
	complete(0);
}
