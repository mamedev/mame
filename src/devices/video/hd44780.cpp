// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Hitachi HD44780 LCD controller

        TODO:
        - dump internal CGROM
        - emulate osc pin, determine video timings and busy flag duration from it

***************************************************************************/

#include "emu.h"
#include "video/hd44780.h"

#define LOG          0

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type HD44780 = &device_creator<hd44780_device>;
const device_type KS0066_F05 = &device_creator<ks0066_f05_device>;


//-------------------------------------------------
//  ROM( hd44780 )
//-------------------------------------------------

ROM_START( hd44780_a00 )
	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "hd44780_a00.bin",    0x0000, 0x1000,  BAD_DUMP CRC(01d108e2) SHA1(bc0cdf0c9ba895f22e183c7bd35a3f655f2ca96f)) // from page 17 of the HD44780 datasheet
ROM_END

ROM_START( ks0066_f05 )
	ROM_REGION( 0x1000, "cgrom", 0 )
	ROM_LOAD( "ks0066_f05.bin",    0x0000, 0x1000,  BAD_DUMP CRC(af9e7bd6) SHA1(0196e871584ee5d370856e7307c0f9d1466e3e51)) // from page 51 of the KS0066 datasheet
ROM_END

//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  hd44780_device - constructor
//-------------------------------------------------

hd44780_device::hd44780_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, HD44780, "HD44780 A00", tag, owner, clock, "hd44780_a00", __FILE__),
	m_pixel_update_func(nullptr),
	m_cgrom(*this, DEVICE_SELF)
{
	set_charset_type(CHARSET_HD44780_A00);
}

hd44780_device::hd44780_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_pixel_update_func(nullptr),
	m_cgrom(*this, DEVICE_SELF)
{
}

ks0066_f05_device::ks0066_f05_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	hd44780_device(mconfig, KS0066_F05, "KS0066 F05", tag, owner, clock, "ks0066_f05", __FILE__)
{
	set_charset_type(CHARSET_KS0066_F05);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *hd44780_device::device_rom_region() const
{
	switch (m_charset_type)
	{
		case CHARSET_HD44780_A00:   return ROM_NAME( hd44780_a00 );
		case CHARSET_KS0066_F05:    return ROM_NAME( ks0066_f05 );
	}

	return nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd44780_device::device_start()
{
	if (!m_cgrom.found())
		m_cgrom.set_target(memregion("cgrom")->base(), 0x1000);

	m_busy_timer = timer_alloc(TIMER_BUSY);
	m_blink_timer = timer_alloc(TIMER_BLINKING);
	m_blink_timer->adjust(attotime::from_msec(409), 0, attotime::from_msec(409));

	// state saving
	save_item(NAME(m_busy_flag));
	save_item(NAME(m_ac));
	save_item(NAME(m_dr));
	save_item(NAME(m_ir));
	save_item(NAME(m_active_ram));
	save_item(NAME(m_display_on));
	save_item(NAME(m_cursor_on));
	save_item(NAME(m_shift_on));
	save_item(NAME(m_blink_on));
	save_item(NAME(m_direction));
	save_item(NAME(m_data_len));
	save_item(NAME(m_num_line));
	save_item(NAME(m_char_size));
	save_item(NAME(m_disp_shift));
	save_item(NAME(m_blink));
	save_item(NAME(m_ddram));
	save_item(NAME(m_cgram));
	save_item(NAME(m_nibble));
	save_item(NAME(m_rs_state));
	save_item(NAME(m_rw_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd44780_device::device_reset()
{
	memset(m_ddram, 0x20, sizeof(m_ddram)); // filled with SPACE char
	memset(m_cgram, 0, sizeof(m_cgram));

	m_ac         = 0;
	m_dr         = 0;
	m_ir         = 0;
	m_active_ram = DDRAM;
	m_display_on = false;
	m_cursor_on  = false;
	m_blink_on   = false;
	m_shift_on   = false;
	m_direction  = 1;
	m_data_len   = 8;
	m_num_line   = 1;
	m_char_size  = 8;
	m_disp_shift = 0;
	m_blink      = false;
	m_nibble     = false;
	m_first_cmd  = true;
	m_rs_state   = 0;
	m_rw_state   = 0;

	set_busy_flag(1520);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void hd44780_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_BUSY:
			m_busy_flag = false;
			break;

		case TIMER_BLINKING:
			m_blink = !m_blink;
			break;
	}
}


//**************************************************************************
//  HELPERS
//**************************************************************************

void hd44780_device::set_charset_type(int type)
{
	m_charset_type = type;
}

void hd44780_device::set_busy_flag(UINT16 usec)
{
	m_busy_flag = true;
	m_busy_timer->adjust( attotime::from_usec( usec ) );
}

void hd44780_device::correct_ac()
{
	if (m_active_ram == DDRAM)
	{
		int max_ac = (m_num_line == 1) ? 0x4f : 0x67;

		if (m_ac > max_ac)
			m_ac -= max_ac + 1;
		else if (m_ac < 0)
			m_ac = max_ac;
		else if (m_num_line == 2 && m_ac > 0x27 && m_ac < 0x40)
			m_ac = 0x40 + (m_ac - 0x28);
	}
	else
		m_ac &= 0x3f;
}

void hd44780_device::update_ac(int direction)
{
	if (m_active_ram == DDRAM && m_num_line == 2 && direction == -1 && m_ac == 0x40)
		m_ac = 0x27;
	else
		m_ac += direction;

	correct_ac();
}

void hd44780_device::shift_display(int direction)
{
	m_disp_shift += direction;

	if (m_disp_shift == 0x50)
		m_disp_shift = 0;
	else if (m_disp_shift == -1)
		m_disp_shift = 0x4f;
}

void hd44780_device::update_nibble(int rs, int rw)
{
	if (m_rs_state != rs || m_rw_state != rw)
	{
		m_rs_state = rs;
		m_rw_state = rw;
		m_nibble = false;
	}

	m_nibble = !m_nibble;
}

inline void hd44780_device::pixel_update(bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state)
{
	if (m_pixel_update_func != nullptr)
	{
		m_pixel_update_func(*this, bitmap, line, pos, y, x, state);
	}
	else
	{
		UINT8 line_height = (m_char_size == 8) ? m_char_size : m_char_size + 1;

		if (m_lines <= 2)
		{
			if (pos < m_chars)
				bitmap.pix16(line * (line_height + 1) + y, pos * 6 + x) = state;
		}
		else if (m_lines <= 4)
		{
			if (pos < m_chars*2)
			{
				if (pos >= m_chars)
				{
					line += 2;
					pos -= m_chars;
				}

				if (line < m_lines)
					bitmap.pix16(line * (line_height + 1) + y, pos * 6 + x) = state;
			}
		}
		else
		{
			fatalerror("%s: use a custom callback for this LCD configuration (%d x %d)\n", tag(), m_lines, m_chars);
		}
	}
}


//**************************************************************************
//  device interface
//**************************************************************************

const UINT8 *hd44780_device::render()
{
	memset(m_render_buf, 0, sizeof(m_render_buf));

	if (m_display_on)
	{
		UINT8 line_size = 80 / m_num_line;

		for (int line = 0; line < m_num_line; line++)
		{
			for (int pos = 0; pos < line_size; pos++)
			{
				UINT16 char_pos = line * 0x40 + ((pos + m_disp_shift) % line_size);

				int char_base;
				if (m_ddram[char_pos] < 0x10)
				{
					// draw CGRAM characters
					if (m_char_size == 8)
						char_base = (m_ddram[char_pos] & 0x07) * 8;
					else
						char_base = ((m_ddram[char_pos] >> 1) & 0x03) * 16;
				}
				else
				{
					// draw CGROM characters
					char_base = m_ddram[char_pos] * 0x10;
				}

				const UINT8 * charset = (m_ddram[char_pos] < 0x10) ? m_cgram : m_cgrom;
				UINT8 *dest = m_render_buf + 16 * (line * line_size + pos);
				memcpy (dest, charset + char_base, m_char_size);

				if (char_pos == m_ac)
				{
					// draw the cursor
					if (m_cursor_on)
						dest[m_char_size - 1] = 0x1f;

					if (!m_blink && m_blink_on)
						memset(dest, 0x1f, m_char_size);
				}
			}
		}
	}

	return m_render_buf;
}

UINT32 hd44780_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	const UINT8 *img = render();

	UINT8 line_size = 80 / m_num_line;

	for (int line = 0; line < m_num_line; line++)
	{
		for (int pos = 0; pos < line_size; pos++)
		{
			const UINT8 *src = img + 16 * (line * line_size + pos);
			for (int y = 0; y < m_char_size; y++)
				for (int x = 0; x < 5; x++)
					pixel_update(bitmap, line, pos, y, x, BIT(src[y], 4 - x));
		}
	}

	return 0;
}

READ8_MEMBER(hd44780_device::read)
{
	switch (offset & 0x01)
	{
		case 0: return control_read(space, 0);
		case 1: return data_read(space, 0);
	}

	return 0;
}

WRITE8_MEMBER(hd44780_device::write)
{
	switch (offset & 0x01)
	{
		case 0: control_write(space, 0, data);  break;
		case 1: data_write(space, 0, data);     break;
	}
}

WRITE8_MEMBER(hd44780_device::control_write)
{
	if (m_data_len == 4)
	{
		update_nibble(0, 0);

		if (m_nibble)
		{
			m_ir = data & 0xf0;
			return;
		}
		else
		{
			m_ir |= ((data >> 4) & 0x0f);
		}
	}
	else
	{
		m_ir = data;
	}

	if (BIT(m_ir, 7))
	{
		// set DDRAM address
		m_active_ram = DDRAM;
		m_ac = m_ir & 0x7f;
		correct_ac();
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': set DDRAM address %x\n", tag(), m_ac);
		return;
	}
	else if (BIT(m_ir, 6))
	{
		// set CGRAM address
		m_active_ram = CGRAM;
		m_ac = m_ir & 0x3f;
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': set CGRAM address %x\n", tag(), m_ac);
		return;
	}
	else if (BIT(m_ir, 5))
	{
		// function set
		if (!m_first_cmd && m_data_len == (BIT(m_ir, 4) ? 8 : 4) && (m_char_size != (BIT(m_ir, 2) ? 10 : 8) || m_num_line != (BIT(m_ir, 3) + 1)))
		{
			logerror("HD44780 '%s': function set cannot be executed after other instructions unless the interface data length is changed\n", tag());
			return;
		}

		m_char_size = BIT(m_ir, 2) ? 10 : 8;
		m_data_len  = BIT(m_ir, 4) ? 8 : 4;
		m_num_line  = BIT(m_ir, 3) + 1;
		correct_ac();
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': char size 5x%d, data len %d, lines %d\n", tag(), m_char_size, m_data_len, m_num_line);
		return;
	}
	else if (BIT(m_ir, 4))
	{
		// cursor or display shift
		int direction = (BIT(m_ir, 2)) ? +1 : -1;

		if (LOG) logerror("HD44780 '%s': %s shift %d\n", tag(), BIT(m_ir, 3) ? "display" : "cursor", direction);

		if (BIT(m_ir, 3))
			shift_display(direction);
		else
			update_ac(direction);

		set_busy_flag(37);
	}
	else if (BIT(m_ir, 3))
	{
		// display on/off control
		m_display_on = BIT(m_ir, 2);
		m_cursor_on  = BIT(m_ir, 1);
		m_blink_on   = BIT(m_ir, 0);
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': display %d, cursor %d, blink %d\n", tag(), m_display_on, m_cursor_on, m_blink_on);
	}
	else if (BIT(m_ir, 2))
	{
		// entry mode set
		m_direction = (BIT(m_ir, 1)) ? +1 : -1;
		m_shift_on  = BIT(m_ir, 0);
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': entry mode set: direction %d, shift %d\n", tag(), m_direction, m_shift_on);
	}
	else if (BIT(m_ir, 1))
	{
		// return home
		if (LOG) logerror("HD44780 '%s': return home\n", tag());

		m_ac         = 0;
		m_active_ram = DDRAM;
		m_direction  = 1;
		m_disp_shift = 0;
		set_busy_flag(1520);
	}
	else if (BIT(m_ir, 0))
	{
		// clear display
		if (LOG) logerror("HD44780 '%s': clear display\n", tag());

		m_ac         = 0;
		m_active_ram = DDRAM;
		m_direction  = 1;
		m_disp_shift = 0;
		memset(m_ddram, 0x20, sizeof(m_ddram));
		set_busy_flag(1520);
	}

	m_first_cmd = false;
}

READ8_MEMBER(hd44780_device::control_read)
{
	if (m_data_len == 4)
	{
		if (!space.debugger_access())
			update_nibble(0, 1);

		if (m_nibble)
			return (m_busy_flag ? 0x80 : 0) | (m_ac & 0x70);
		else
			return (m_ac << 4) & 0xf0;
	}
	else
	{
		return (m_busy_flag ? 0x80 : 0) | (m_ac & 0x7f);
	}
}

WRITE8_MEMBER(hd44780_device::data_write)
{
	if (m_busy_flag)
	{
		logerror("HD44780 '%s': Ignoring data write %02x due of busy flag\n", tag(), data);
		return;
	}

	if (m_data_len == 4)
	{
		update_nibble(1, 0);

		if (m_nibble)
		{
			m_dr = data & 0xf0;
			return;
		}
		else
		{
			m_dr |= ((data >> 4) & 0x0f);
		}
	}
	else
	{
		m_dr = data;
	}

	if (LOG) logerror("HD44780 '%s': %sRAM write %x %x '%c'\n", tag(), m_active_ram == DDRAM ? "DD" : "CG", m_ac, m_dr, isprint(m_dr) ? m_dr : '.');

	if (m_active_ram == DDRAM)
		m_ddram[m_ac] = m_dr;
	else
		m_cgram[m_ac] = m_dr;

	update_ac(m_direction);
	if (m_shift_on)
		shift_display(m_direction);
	set_busy_flag(41);
}

READ8_MEMBER(hd44780_device::data_read)
{
	UINT8 data = (m_active_ram == DDRAM) ? m_ddram[m_ac] : m_cgram[m_ac];

	if (LOG) logerror("HD44780 '%s': %sRAM read %x %c\n", tag(), m_active_ram == DDRAM ? "DD" : "CG", m_ac, data);

	if (m_data_len == 4)
	{
		if (!space.debugger_access())
			update_nibble(1, 1);

		if (m_nibble)
			return data & 0xf0;
		else
			data = (data << 4) & 0xf0;
	}

	if (!space.debugger_access())
	{
		update_ac(m_direction);
		set_busy_flag(41);
	}

	return data;
}
