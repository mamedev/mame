/***************************************************************************

        Hitachi HD44780 LCD controller

        TODO:
        - dump internal CGROM

***************************************************************************/

#include "emu.h"
#include "video/hd44780.h"

#define LOG          0

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type HD44780 = &device_creator<hd44780_device>;


//-------------------------------------------------
//  ROM( hd44780 )
//-------------------------------------------------

ROM_START( hd44780 )
	ROM_REGION( 0x0860, "cgrom", 0 )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))
ROM_END


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  hd44780_device - constructor
//-------------------------------------------------

hd44780_device::hd44780_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, HD44780, "HD44780", tag, owner, clock),
	m_pixel_update_func(NULL)
{
	m_shortname = "hd44780";
}

hd44780_device::hd44780_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock),
	m_pixel_update_func(NULL)
{
	m_shortname = "hd44780";
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *hd44780_device::device_rom_region() const
{
	return ROM_NAME( hd44780 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd44780_device::device_start()
{
	if (region())
		m_cgrom = (UINT8*)(*region());
	else
		m_cgrom = (UINT8*)(*memregion("cgrom"));

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
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd44780_device::device_reset()
{
	memset(m_ddram, 0x20, sizeof(m_ddram)); // can't use 0 here as it would show CGRAM instead of blank space on a soft reset
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

	set_busy_flag(1520);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void hd44780_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
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

void hd44780_device::set_busy_flag(UINT16 usec)
{
	m_busy_flag = true;
	m_busy_timer->adjust( attotime::from_usec( usec ) );
}

void hd44780_device::update_ac(int direction)
{
	if (m_active_ram == DDRAM)
	{
		if(direction == 1)
		{
			if(m_num_line == 2 && m_ac == 0x27)
				m_ac = 0x40;
			else if((m_num_line == 2 && m_ac == 0x67) || (m_num_line == 1 && m_ac == 0x4f))
				m_ac = 0x00;
			else
				m_ac = (m_ac + direction) & 0x7f;
		}
		else
		{
			if(m_num_line == 2 && m_ac == 0x00)
				m_ac = 0x67;
			else if(m_num_line == 1 && m_ac == 0x00)
				m_ac = 0x4f;
			else if(m_num_line == 2 && m_ac == 0x40)
				m_ac = 0x27;
			else
				m_ac = (m_ac + direction) & 0x7f;
		}
	}
	else
	{
		m_ac = (m_ac + direction) & 0x3f;
	}
}

inline void hd44780_device::pixel_update(bitmap_ind16 &bitmap, UINT8 line, UINT8 pos, UINT8 y, UINT8 x, int state)
{
	if (m_pixel_update_func != NULL)
	{
		m_pixel_update_func(*this, bitmap, line, pos, y, x, state);
	}
	else
	{
		if (m_lines <= 2)
		{
			if (pos < m_chars)
				bitmap.pix16(line * (m_char_size+1) + y, pos * 6 + x) = state;
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
					bitmap.pix16(line * (m_char_size+1) + y, pos * 6 + x) = state;
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

UINT32 hd44780_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (m_display_on)
	{
		UINT8 line_size = 80 / m_num_line;

		for (int line=0; line<m_num_line; line++)
		{
			UINT8 line_base = line * 0x40;

			for (int pos=0; pos<line_size; pos++)
			{
				INT16 char_pos = line_base + pos + m_disp_shift;

				while (char_pos < 0 || (char_pos - line_base) >= line_size)
				{
					if (char_pos < 0)
						char_pos += line_size;
					else if (char_pos - line_base >= line_size)
						char_pos -= line_size;
				}

				int char_base = 0;
				if (m_ddram[char_pos] < 0x10)
				{
					// draw CGRAM characters
					if (m_char_size == 8)
						char_base = (m_ddram[char_pos] & 0x07) * 8;
					else
						char_base = (m_ddram[char_pos] & 0x03) * 16;
				}
				else
				{
					// draw CGROM characters
					if (m_ddram[char_pos] < 0xe0)
						char_base = m_ddram[char_pos] * 8;
					else
						char_base = 0x700 + ((m_ddram[char_pos] - 0xe0) * 11);
				}

				for (int y=0; y<m_char_size; y++)
				{
					UINT8 * charset = (m_ddram[char_pos] < 0x10) ? m_cgram : m_cgrom;

					for (int x=0; x<5; x++)
					{
						if (m_ddram[char_pos] >= 0xe0 || y < 8)
							pixel_update(bitmap, line, pos, y, x, BIT(charset[char_base + y], 4 - x));
						else
							pixel_update(bitmap, line, pos, y, x, 0);
					}
				}

				// if is the correct position draw cursor and blink
				if (char_pos == m_ac)
				{
					// draw the cursor
					if (m_cursor_on)
						for (int x=0; x<5; x++)
							pixel_update(bitmap, line, pos, m_char_size - 1, x, 1);

					if (!m_blink && m_blink_on)
						for (int y=0; y<(m_char_size - 1); y++)
							for (int x=0; x<5; x++)
								pixel_update(bitmap, line, pos, y, x, 1);
				}
			}
		}
	}

	return 0;
}

READ8_MEMBER(hd44780_device::read)
{
	switch(offset & 0x01)
	{
		case 0: return control_read(space, 0);
		case 1: return data_read(space, 0);
	}

	return 0;
}

WRITE8_MEMBER(hd44780_device::write)
{
	switch(offset & 0x01)
	{
		case 0: control_write(space, 0, data);  break;
		case 1: data_write(space, 0, data);     break;
	}
}

WRITE8_MEMBER(hd44780_device::control_write)
{
	if (m_data_len == 4)
	{
		m_nibble = !m_nibble;

		if (m_nibble)
		{
			m_ir = data & 0xf0;
			return;
		}
		else
		{
			m_ir |= ((data>>4) & 0x0f);
		}
	}
	else
	{
		m_ir = data;
	}

	if (BIT(m_ir, 7))           // set DDRAM address
	{
		m_active_ram = DDRAM;
		m_ac = m_ir & 0x7f;
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': set DDRAM address %x\n", tag(), m_ac);
	}
	else if (BIT(m_ir, 6))      // set CGRAM address
	{
		m_active_ram = CGRAM;
		m_ac = m_ir & 0x3f;
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': set CGRAM address %x\n", tag(), m_ac);
	}
	else if (BIT(m_ir, 5))      // function set
	{
		if (!m_first_cmd && m_data_len == (BIT(m_ir, 4) ? 8 : 4) && (m_char_size != (BIT(m_ir, 2) ? 10 : 8) || m_num_line != (BIT(m_ir, 3) + 1)))
		{
			logerror("HD44780 '%s': function set cannot be executed after other instructions unless the interface data length is changed\n", tag());
			return;
		}

		m_char_size = BIT(m_ir, 2) ? 10 : 8;
		m_data_len  = BIT(m_ir, 4) ? 8 : 4;
		m_num_line  = BIT(m_ir, 3) + 1;
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': char size 5x%d, data len %d, lines %d\n", tag(), m_char_size, m_data_len, m_num_line);
		return;
	}
	else if (BIT(m_ir, 4))      // cursor or display shift
	{
		int direct = (BIT(m_ir, 2)) ? +1 : -1;

		if (LOG) logerror("HD44780 '%s': %s shift %d\n", tag(), BIT(m_ir, 3) ? "display" : "cursor",  direct);

		if (BIT(m_ir, 3))
			m_disp_shift += direct;
		else
			update_ac(direct);

		set_busy_flag(37);
	}
	else if (BIT(m_ir, 3))      // display on/off control
	{
		m_display_on = BIT(m_ir, 2);
		m_cursor_on  = BIT(m_ir, 1);
		m_blink_on   = BIT(m_ir, 0);
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': display %d, cursor %d, blink %d\n", tag(), m_display_on, m_cursor_on, m_blink_on);
	}
	else if (BIT(m_ir, 2))      // entry mode set
	{
		m_direction = (BIT(m_ir, 1)) ? +1 : -1;
		m_shift_on  = BIT(m_ir, 0);
		set_busy_flag(37);

		if (LOG) logerror("HD44780 '%s': entry mode set: direction %d, shift %d\n", tag(), m_direction, m_shift_on);
	}
	else if (BIT(m_ir, 1))      // return home
	{
		if (LOG) logerror("HD44780 '%s': return home\n", tag());

		m_ac         = 0;
		m_active_ram = DDRAM;
		m_direction  = 1;
		m_disp_shift = 0;
		set_busy_flag(1520);
	}
	else if (BIT(m_ir, 0))      // clear display
	{
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
			m_nibble = !m_nibble;

		if (m_nibble)
			return (m_busy_flag ? 0x80 : 0) | (m_ac & 0x70);
		else
			return (m_ac<<4) & 0xf0;
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
		m_nibble = !m_nibble;

		if (m_nibble)
		{
			m_dr = data & 0xf0;
			return;
		}
		else
		{
			m_dr |= ((data>>4) & 0x0f);
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
		m_disp_shift += m_direction;
	set_busy_flag(41);
}

READ8_MEMBER(hd44780_device::data_read)
{
	UINT8 data = (m_active_ram == DDRAM) ? m_ddram[m_ac] : m_cgram[m_ac];

	if (LOG) logerror("HD44780 '%s': %sRAM read %x %c\n", tag(), m_active_ram == DDRAM ? "DD" : "CG", m_ac, data);

	if (m_data_len == 4)
	{
		if (!space.debugger_access())
			m_nibble = !m_nibble;

		if (m_nibble)
			return data & 0xf0;
		else
			data = (data<<4) & 0xf0;
	}

	if (!space.debugger_access())
	{
		update_ac(m_direction);
		set_busy_flag(41);
	}

	return data;
}
