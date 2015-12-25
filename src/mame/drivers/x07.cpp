// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Canon X-07

    Driver by Sandro Ronco based on X-07 emu by J.Brigaud

    TODO:
    - move T6834 in a device
    - better emulation of the i/o ports
    - external video (need X-720 dump)
    - serial port

    Memory Map

    0x0000 - 0x1fff   Internal RAM
    0x2000 - 0x3fff   External RAM Card
    0x4000 - 0x5fff   Extension ROM/RAM
    0x6000 - 0x7fff   ROM Card
    0x8000 - 0x97ff   Video RAM
    0x9800 - 0x9fff   ?
    0xa000 - 0xafff   TV ROM (no dump)
    0xb000 - 0xffff   ROM

    CPU was actually a NSC800 (Z80 compatible)
    More info: http://www.silicium.org/oldskool/calc/x07/

****************************************************************************/

#include "includes/x07.h"
#include "softlist.h"

/***************************************************************************
    T6834 IMPLEMENTATION
***************************************************************************/

void x07_state::t6834_cmd (UINT8 cmd)
{
	switch (cmd)
	{
	case 0x00:  //NOP???
		break;

	case 0x01:  //DATA$ TIME$ read
		{
			system_time systime;
			machine().current_datetime(systime);
			m_out.data[m_out.write++] = (systime.local_time.year>>8) & 0xff;
			m_out.data[m_out.write++] = systime.local_time.year & 0xff;
			m_out.data[m_out.write++] = systime.local_time.month + 1;
			m_out.data[m_out.write++] = systime.local_time.mday;
			m_out.data[m_out.write++] = ~(((0x01 << (7 - systime.local_time.weekday)) - 1) & 0xff);
			m_out.data[m_out.write++] = systime.local_time.hour;
			m_out.data[m_out.write++] = systime.local_time.minute;
			m_out.data[m_out.write++] = systime.local_time.second;
		}
		break;

	case 0x02:  //STICK
		{
			UINT8 data;

			switch (ioport("S1")->read() & 0x3c)
			{
				case 0x04:      data = 0x33;    break;  //right
				case 0x08:      data = 0x37;    break;  //left
				case 0x10:      data = 0x31;    break;  //up
				case 0x20:      data = 0x35;    break;  //down
				default:        data = 0x30;    break;
			}
			m_out.data[m_out.write++] = data;
		}
		break;

	case 0x03:  //STRIG(0)
		{
			m_out.data[m_out.write++] = (ioport("S6")->read() & 0x20 ? 0x00 : 0xff);
		}
		break;

	case 0x04:  //STRIG(1)
		{
			m_out.data[m_out.write++] = (ioport("S1")->read() & 0x40 ? 0x00 : 0xff);
		}
		break;

	case 0x05:  //T6834 RAM read
		{
			UINT16 address;
			UINT8 data;
			address = m_in.data[m_in.read++];
			address |= (m_in.data[m_in.read++] << 8);

			if(address == 0xc00e)
				data = 0x0a;
			else if(address == 0xd000)
				data = ioport("BATTERY")->read();
			else
				data = m_t6834_ram[address & 0x7ff];

			m_out.data[m_out.write++] = data;
		}
		break;

	case 0x06:  //T6834 RAM write
		{
			UINT16 address;
			UINT8 data;
			data = m_in.data[m_in.read++];
			address = m_in.data[m_in.read++];
			address |= (m_in.data[m_in.read++] << 8);

			m_t6834_ram[address & 0x7ff] = data;
		}
		break;

	case 0x07:  //scroll set
		{
			m_scroll_min = m_in.data[m_in.read++];
			m_scroll_max = m_in.data[m_in.read++];
		}
		break;

	case 0x08:  //scroll exec
		{
			if(m_scroll_min <= m_scroll_max && m_scroll_max < 4)
			{
				for(int i = m_scroll_min * 8; i < m_scroll_max * 8; i++)
					memcpy(&m_lcd_map[i][0], &m_lcd_map[i + 8][0], 120);

				for(int i = m_scroll_max * 8; i < (m_scroll_max + 1) * 8; i++)
					memset(&m_lcd_map[i][0], 0, 120);
			}
		}
		break;

	case 0x09:  //line clear
		{
			UINT8 line = m_in.data[m_in.read++] & 3;
			for(UINT8 l = line * 8; l < (line + 1) * 8; l++)
				memset(&m_lcd_map[l][0], 0, 120);
		}
		break;

	case 0x0a:  //DATA$ TIME$ write
		break;

	case 0x0b:  //calendar
		{
				system_time systime;
				machine().current_datetime(systime);
				m_out.data[m_out.write++] = systime.local_time.weekday;
		}
		break;

	case 0x0c:  //ALM$ write
		{
			for(auto & elem : m_alarm)
				elem = m_in.data[m_in.read++];
		}
		break;

	case 0x0d:  //buzzer on
	case 0x0e:  //buzzer off
		break;

	case 0x0f:  //read LCD line
		{
			UINT8 line = m_in.data[m_in.read++];
			for(int i = 0; i < 120; i++)
				m_out.data[m_out.write++] = (line < 32) ? m_lcd_map[line][i] : 0;
		}
		break;

	case 0x10:  //read LCD point
		{
			UINT8 x = m_in.data[m_in.read++];
			UINT8 y = m_in.data[m_in.read++];
			if(x < 120 && y < 32)
				m_out.data[m_out.write++] = (m_lcd_map[y][x] ? 0xff : 0);
			else
				m_out.data[m_out.write++] = 0;
		}
		break;

	case 0x11:  //PSET
		{
			UINT8 x = m_in.data[m_in.read++];
			UINT8 y = m_in.data[m_in.read++];
			draw_point(x, y, 1);
		}
		break;

	case 0x12:  //PRESET
		{
			UINT8 x = m_in.data[m_in.read++];
			UINT8 y = m_in.data[m_in.read++];
			draw_point(x, y, 0);
		}
		break;

	case 0x13:  //PEOR
		{
			UINT8 x = m_in.data[m_in.read++];
			UINT8 y = m_in.data[m_in.read++];
			if(x < 120 && y < 32)
				m_lcd_map[y][x] = !m_lcd_map[y][x];
		}
		break;

	case 0x14:  //Line
		{
			UINT8 delta_x, delta_y, step_x, step_y, next_x, next_y, p1, p2, p3, p4;
			INT16 frac;
			next_x = p1 = m_in.data[m_in.read++];
			next_y = p2 = m_in.data[m_in.read++];
			p3 = m_in.data[m_in.read++];
			p4 = m_in.data[m_in.read++];
			delta_x = abs(p3 - p1) * 2;
			delta_y = abs(p4 - p2) * 2;
			step_x = (p3 < p1) ? -1 : 1;
			step_y = (p4 < p2) ? -1 : 1;

			if(delta_x > delta_y)
			{
				frac = delta_y - delta_x / 2;
				while(next_x != p3)
				{
					if(frac >= 0)
					{
						next_y += step_y;
						frac -= delta_x;
					}
					next_x += step_x;
					frac += delta_y;
					draw_point(next_x, next_y, 0x01);
				}
			}
			else {
				frac = delta_x - delta_y / 2;
				while(next_y != p4)
				{
					if(frac >= 0)
					{
						next_x += step_x;
						frac -= delta_y;
					}
					next_y += step_y;
					frac += delta_x;
					draw_point(next_x, next_y, 0x01);
				}
			}
			draw_point(p1, p2, 0x01);
			draw_point(p3, p4, 0x01);
		}
		break;

	case 0x15:  //Circle
		{
			UINT8 p1 = m_in.data[m_in.read++];
			UINT8 p2 = m_in.data[m_in.read++];
			UINT8 p3 = m_in.data[m_in.read++];

			for(int x = 0, y = p3; x <= sqrt((double)(p3 * p3) / 2) ; x++)
			{
				/*
				 * The old code produced results most likely not intended:
				 * UINT32 d1 = (x * x + y * y) - p3 * p3;
				 * UINT32 d2 = (x * x + (y - 1) * (y - 1)) - p3 * p3;
				 * if(abs((double)d1) > abs((double)d2))
				 *
				 * (double)(-1) = 4294967294.000000
				 * abs((double)(-1)) = -2147483648;
				 *
				 * Therefore changed.
				 */
				INT32 d1 = (x * x + y * y) - p3 * p3;
				INT32 d2 = (x * x + (y - 1) * (y - 1)) - p3 * p3;
				if (abs(d1) > abs(d2))
					y--;
				draw_point(x + p1, y + p2, 0x01);
				draw_point(x + p1, -y + p2, 0x01);
				draw_point(-x + p1, y + p2, 0x01);
				draw_point(-x + p1, -y + p2, 0x01);
				draw_point(y + p1, x + p2, 0x01);
				draw_point(y + p1, -x + p2, 0x01);
				draw_point(-y + p1, x + p2, 0x01);
				draw_point(-y + p1, -x + p2, 0x01);
			}
		}
		break;

	case 0x16:  //UDK write
		{
			UINT8 pos = m_in.data[m_in.read++] - 1;
			UINT8 udk_size = (pos != 5 && pos != 11) ? 0x2a : 0x2e;

			for(int i = 0; i < udk_size; i++)
			{
				UINT8 udk_char = m_in.data[m_in.read++];
				m_t6834_ram[udk_offset[pos] + i] = udk_char;
				if(!udk_char)   break;
			}
		}
		break;

	case 0x17:  //UDK read
		{
			UINT8 pos = m_in.data[m_in.read++] - 1;
			UINT8 udk_size = (pos != 5 && pos != 11) ? 0x2a : 0x2e;

			for(int i = 0; i < udk_size; i++)
			{
				UINT8 udk_char = m_t6834_ram[udk_offset[pos] + i];
				m_out.data[m_out.write++] = udk_char;
				if(!udk_char)   break;
			}
		}
		break;

	case 0x18:  //UDK on
	case 0x19:  //UDK off
		m_udk_on = !BIT(cmd,0);
		break;

	case 0x1a:  //UDC write
		{
			UINT8 udc_code = m_in.data[m_in.read++];

			if(udc_code>=128 && udc_code<=159)
				for(int i = 0; i < 8; i++)
					m_t6834_ram[(udc_code<<3) + i - 0x200] = m_in.data[m_in.read++];
			else if(udc_code>=224)
				for(int i = 0; i < 8; i++)
					m_t6834_ram[(udc_code<<3) + i - 0x400] = m_in.data[m_in.read++];
		}
		break;

	case 0x1b:  //UDC read
		{
			UINT16 address = m_in.data[m_in.read++] << 3;
			for(int i = 0; i < 8; i++)
				m_out.data[m_out.write++] = get_char(address + i);
		}
		break;
	case 0x1c:  //UDC Init
		{
			memcpy(m_t6834_ram + 0x200, (UINT8*)memregion("gfx1")->base() + 0x400, 0x100);
			memcpy(m_t6834_ram + 0x300, (UINT8*)memregion("gfx1")->base() + 0x700, 0x100);
		}
		break;

	case 0x1d:  //start program write
		{
			for(int i = 0; i < 0x80; i++)
			{
				UINT8 sp_char = m_in.data[m_in.read++];
				m_t6834_ram[0x500 + i] = sp_char;
				if (!sp_char) break;
			}
		}
		break;

	case 0x1e:  //start program write cont
		{
			for(int i = (int)strlen((char*)&m_t6834_ram[0x500]); i < 0x80; i++)
			{
				UINT8 sp_char = m_in.data[m_in.read++];
				m_t6834_ram[0x500 + i] = sp_char;
				if (!sp_char) break;
			}
		}
		break;

	case 0x1f:  //start program on
	case 0x20:  //start program off
		m_sp_on = BIT(cmd, 0);
		break;

	case 0x21:  //start program read
		{
			for(int i = 0; i < 0x80; i++)
			{
				UINT8 sp_data = m_t6834_ram[0x500 + i];
				m_out.data[m_out.write++] = sp_data;
				if (!sp_data) break;
			}
		}
		break;

	case 0x22: //ON state
		m_out.data[m_out.write++] = 0x04 | (m_sleep<<6) | m_warm_start;
		break;

	case 0x23:  //OFF
		m_warm_start = 1;
		m_sleep = 0;
		m_lcd_on = 0;
		break;

	case 0x24:  //locate
		{
			UINT8 x = m_in.data[m_in.read++];
			UINT8 y = m_in.data[m_in.read++];
			UINT8 char_code = m_in.data[m_in.read++];
			m_locate.on = (m_locate.x != x || m_locate.y != y);
			m_locate.x = m_cursor.x = x;
			m_locate.y = m_cursor.y = y;

			if(char_code)
				draw_char(x, y, char_code);
		}
		break;

	case 0x25:  //cursor on
	case 0x26:  //cursor off
		m_cursor.on = BIT(cmd, 0);
		break;

	case 0x27:  //test key
		{
			static const char *const lines[] = {"S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "BZ", "A1"};
			UINT16 matrix;
			UINT8 data = 0;
			matrix = m_in.data[m_in.read++];
			matrix |= (m_in.data[m_in.read++] << 8);

			for (int i=0 ;i<10; i++)
				if (matrix & (1<<i))
					data |= ioport(lines[i])->read();

			m_out.data[m_out.write++] = data;
		}
		break;

	case 0x28:  //test chr
		{
			UINT8 idx = kb_get_index(m_in.data[m_in.read++]);
			m_out.data[m_out.write++] = (ioport(x07_keycodes[idx].tag)->read() & x07_keycodes[idx].mask) ? 0x00 : 0xff;
		}
		break;

	case 0x29:  //init sec
	case 0x2a:  //init date
		break;

	case 0x2b:  //LCD off
	case 0x2c:  //LCD on
		m_lcd_on = !BIT(cmd,0);
		break;

	case 0x2d:  //KB buffer clear
		memset(m_t6834_ram + 0x400, 0, 0x100);
		m_kb_size = 0;
		break;

	case 0x2e:  //CLS
		memset(m_lcd_map, 0, sizeof(m_lcd_map));
		break;

	case 0x2f:  //home
		m_cursor.x = m_cursor.y = 0;
		break;

	case 0x30:  //draw UDK on
	case 0x31:  //draw UDK off
		{
			m_draw_udk = !BIT(cmd,0);

			if (m_draw_udk)
				draw_udk();
			else
				for(UINT8 l = 3 * 8; l < (3 + 1) * 8; l++)
					memset(&m_lcd_map[l][0], 0, 120);
		}
		break;

	case 0x32:  //repeat key on
	case 0x33:  //repeat key off
		m_repeat_key = !BIT(cmd,0);
		break;

	case 0x34:  //UDK KANA
		break;

	case 0x35:  //UDK cont write
		{
			UINT8 pos = m_in.data[m_in.read++] - 1;
			UINT8 udk_size = (pos != 5 && pos != 11) ? 0x2a : 0x2e;

			for(int i = (int)strlen((char*)&m_t6834_ram[udk_offset[pos]]); i < udk_size; i++)
			{
				UINT8 udk_char = m_in.data[m_in.read++];
				m_t6834_ram[udk_offset[pos] + i] = udk_char;
				if(!udk_char)   break;
			}
		}
		break;

	case 0x36:  //alarm read
		{
			for(auto & elem : m_alarm)
				m_out.data[m_out.write++] = elem;
		}
		break;

	case 0x37: // buzzer zero
		m_out.data[m_out.write++] = 0xff;
		break;

	case 0x38:  //click off
	case 0x39:  //click on
		break;

	case 0x3a:  //Locate Close
		break;

	case 0x3b: // keyboard on
	case 0x3c: // keyboard off
		m_kb_on = BIT(cmd, 0);
		break;

	case 0x3d:  //run start program after power on
	case 0x3e:  //run start program before power off
		break;

	case 0x3f: //Sleep
		m_warm_start = 1;
		m_lcd_on = 0;
		m_sleep = 1;
		break;

	case 0x40:  //UDK init
		{
			memset(m_t6834_ram, 0, 0x200);
			for(int i = 0; i < 12; i++)
				strcpy((char*)m_t6834_ram + udk_offset[i], udk_ini[i]);
		}
		break;

	case 0x41:  //char wrire
		{
			for(int cy = 0; cy < 8; cy++)
			{
				UINT8 cl = m_in.data[m_in.read++];

				for(int cx = 0; cx < 6; cx++)
					m_lcd_map[m_cursor.y * 8 + cy][m_cursor.x * 6 + cx] = (cl & (0x80>>cx)) ? 1 : 0;
			}
		}
		break;

	case 0x42: //char read
		{
			for(int cy = 0; cy < 8; cy++)
			{
				UINT8 cl = 0x00;

				for(int cx = 0; cx < 6; cx++)
					cl |= (m_lcd_map[m_cursor.y * 8 + cy][m_cursor.x * 6 + cx] != 0) ? (1<<(7-cx)) : 0;

				m_out.data[m_out.write++] = cl;
			}
		}
		break;

	case 0x43:  //ScanR
	case 0x44:  //ScanL
		{
			m_out.data[m_out.write++] = 0;
			m_out.data[m_out.write++] = 0;
		}
		break;

	case 0x45:  //TimeChk
	case 0x46:  //AlmChk
		m_out.data[m_out.write++] = 0;
		break;
	default:
		logerror( "T6834 unimplemented command %02x encountered\n", cmd );
	}
}


void x07_state::t6834_r ()
{
	m_out.read++;
	m_regs_r[2] &= 0xfe;
	if(m_out.write > m_out.read)
	{
		m_regs_r[0]  = 0x40;
		m_regs_r[1] = m_out.data[m_out.read];
		m_regs_r[2] |= 0x01;
		m_maincpu->set_input_line(NSC800_RSTA, ASSERT_LINE);
		m_rsta_clear->adjust(attotime::from_msec(50));
	}
}


void x07_state::t6834_w ()
{
	if (!m_in.write)
	{
		if (m_locate.on && ((m_regs_w[1] & 0x7F) != 0x24) && ((m_regs_w[1]) >= 0x20) && ((m_regs_w[1]) < 0x80))
		{
			m_cursor.x++;
			draw_char(m_cursor.x, m_cursor.y, m_regs_w[1]);
		}
		else
		{
			m_locate.on = 0;

			if ((m_regs_w[1] & 0x7f) < 0x47)
			{
				m_in.data[m_in.write++] = m_regs_w[1] & 0x7f;
			}
		}
	}
	else
	{
		m_in.data[m_in.write++] = m_regs_w[1];

		if (m_in.write == 2)
		{
			if (m_in.data[m_in.read] == 0x0c && m_regs_w [1] == 0xb0)
			{
				memset(m_lcd_map, 0, sizeof(m_lcd_map));
				m_in.write = 0;
				m_in.read = 0;
				m_in.data[m_in.write++] = m_regs_w[1] & 0x7f;
			}

			if (m_in.data[m_in.read] == 0x07 && m_regs_w [1] > 4)
			{
				m_in.write = 0;
				m_in.read = 0;
				m_in.data[m_in.write++] = m_regs_w[1] & 0x7f;
			}
		}
	}

	if (m_in.write)
	{
		UINT8 cmd_len = t6834_cmd_len[m_in.data[m_in.read]];
		if(cmd_len & 0x80)
		{
			if((cmd_len & 0x7f) < m_in.write && !m_regs_w[1])
				cmd_len = m_in.write;
		}

		if(m_in.write == cmd_len)
		{
			m_out.write = 0;
			m_out.read = 0;
			t6834_cmd(m_in.data[m_in.read++]);
			m_in.write = 0;
			m_in.read = 0;
			if(m_out.write)
			{
				m_regs_r[0]  = 0x40;
				m_regs_r[1] = m_out.data[m_out.read];
				m_regs_r[2] |= 0x01;
				m_maincpu->set_input_line(NSC800_RSTA, ASSERT_LINE);
				m_rsta_clear->adjust(attotime::from_msec(50));
			}
		}
	}
}

void x07_state::cassette_r()
{
	m_regs_r[6] &= ~2;
}

void x07_state::cassette_w()
{
	m_regs_r[6] &= ~1;
	m_cass_data = m_regs_w[7];
}

TIMER_CALLBACK_MEMBER(x07_state::cassette_tick)
{
	m_cass_clk++;
}

TIMER_CALLBACK_MEMBER(x07_state::cassette_poll)
{
	if ((m_cassette->get_state() & 0x03) == CASSETTE_PLAY)
		cassette_load();
	else if ((m_cassette->get_state() & 0x03) == CASSETTE_RECORD)
		cassette_save();
}

void x07_state::cassette_load()
{
	int cass = (m_cassette->input() >= 0) ? +1 : -1;
	if (cass > 0 && m_cass_state < 0)
	{
		if ((m_cass_clk & 0x7f) >= 4 && (m_cass_clk & 0x7f) <= 6)
		{
			if (m_cass_clk & 0x80)
			{
				m_cass_clk = 0;
				receive_bit(1);
			}
			else
			{
				m_cass_clk = 0x80;
			}
		}
		else if ((m_cass_clk & 0x7f) >= 9 && (m_cass_clk & 0x7f) <= 11)
		{
			m_cass_clk = 0;
			receive_bit(0);
		}
		else
		{
			m_cass_clk = 0;
			logerror("Invalid data: %d %f\n", (m_cass_clk & 0x7f), m_cassette->get_position());
		}

		m_cass_tick->adjust(attotime::from_hz(12000), 0, attotime::from_hz(12000));
	}

	m_cass_state = cass;
}


void x07_state::cassette_save()
{
	int cass = m_cass_state;

	if (m_cass_clk % 10 == 0)
	{
		if (m_bit_count < 4)
		{
			switch (m_bit_count & 3)
			{
				case 0:     case 1:     cass = +1;  break;
				case 2:     case 3:     cass = -1;  break;
			}

			m_bit_count++;
		}
		else if (m_bit_count < 36)
		{
			switch (m_bit_count & 3)
			{
				case 0:     cass = +1;  break;
				case 1:     cass = (m_cass_data & 1) ? -1 : +1; break;
				case 2:     cass = (m_cass_data & 1) ? +1 : -1; break;
				case 3:     cass = -1;  m_cass_data >>= 1;      break;
			}

			m_bit_count++;
		}
		else if (m_bit_count < 48)
		{
			switch (m_bit_count & 3)
			{
				case 0:     case 2:     cass = +1;  break;
				case 1:     case 3:     cass = -1;  break;
			}

			if (m_bit_count == 47)
				m_regs_r[6] |= 1;

			m_bit_count++;
		}
		else
		{
			cass = (m_cass_state > 0) ? -1 : +1;
		}

		m_cassette->output( cass );
	}

	// finish the current cycle before start the next
	if ((m_cass_state <= 0) && !(m_regs_r[6] & 1) && m_bit_count >= 48)
		m_bit_count = 0;

	m_cass_state = cass;
	m_cass_clk++;
}

void x07_state::receive_bit(int bit)
{
	if (m_bit_count == 0)
	{
		// wait for start bit
		if (bit == 0)
			m_bit_count++;
	}
	else if (m_bit_count < 9)
	{
		m_cass_data = (m_cass_data>>1) | (bit<<7);
		m_bit_count++;
	}
	else if (m_bit_count < 12)
	{
		if (bit != 1)
			logerror("Invalid stop bit: %f\n", m_cassette->get_position());

		m_bit_count++;
	}

	// every byte take 12 bit
	if (m_bit_count == 12)
	{
		//printf("data: %02x %f\n", m_cass_data, m_cassette->get_position());
		m_regs_r[6] |= 2;
		m_regs_r[7] = m_cass_data;
		m_cass_data = 0;
		m_bit_count = 0;

		m_maincpu->set_input_line(NSC800_RSTB, ASSERT_LINE);
		m_rstb_clear->adjust(attotime::from_usec(200));

	}
}


/****************************************************
    this function emulate the color printer X-710
    only the text functions are emulated
****************************************************/
void x07_state::printer_w()
{
	UINT16 char_pos = 0;
//  UINT16 text_color = 0;
//  UINT16 text_size = 1;

	if (m_regs_r[4] & 0x20)
		m_prn_char_code |= 1;

	m_prn_sendbit++;

	if (m_prn_sendbit == 8)
	{
		if (m_prn_char_code)
		{
			m_prn_buffer[m_prn_size++] = m_prn_char_code;

			if (m_prn_buffer[m_prn_size - 2] == 0x4f && m_prn_buffer[m_prn_size - 1] == 0xaf)
			{
				if (m_prn_buffer[0] == 0xff && m_prn_buffer[1] == 0xb7)
				{
					for (int i = 2; i < m_prn_size - 2; i++)
					{
/*
                        if (m_prn_buffer[i - 1] == 0x4f && m_prn_buffer[i] == 0x3d)
                            text_color = printer_charcode[m_prn_buffer[i + 1]] - 0x30;

                        if (m_prn_buffer[i - 1] == 0x4f && m_prn_buffer[i] == 0x35)
                        {
                            if (m_prn_buffer[i + 2] == 0x4f)
                                text_size = printer_charcode[m_prn_buffer[i + 1]] - 0x2f;
                            else
                                text_size = 0x0a + (printer_charcode[m_prn_buffer[i + 2]] - 0x2f);
                        }
*/
						if (m_prn_buffer[i - 1] == 0x4f && m_prn_buffer[i] == 0x77)
						{
							char_pos = i + 1 ;
							break;
						}
					}
				}

				//send the chars to the printer, color and size are not used
				for (int i = char_pos ;i < m_prn_size ; i++)
					m_printer->output(printer_charcode[m_prn_buffer[i]]);

				//clears the print buffer
				memset(m_prn_buffer, 0, sizeof(m_prn_buffer));
				m_prn_size = 0;
			}
		}

		m_prn_sendbit = 0;
		m_prn_char_code = 0;
		m_regs_r[2] |= 0x80;
	}
	else
		m_prn_char_code <<= 1;
}

inline UINT8 x07_state::kb_get_index(UINT8 char_code)
{
	for(UINT8 i=0 ; i< ARRAY_LENGTH(x07_keycodes); i++)
		if (x07_keycodes[i].codes[0] == char_code)
			return i;

	return 0;
}

inline UINT8 x07_state::get_char(UINT16 pos)
{
	UINT8 code = pos>>3;

	if(code>=128 && code<=159)      //UDC 0
	{
		return m_t6834_ram[pos - 0x200];
	}
	else if(code>=224)              //UDC 1
	{
		return m_t6834_ram[pos - 0x400];
	}
	else                            //charset
	{
		return memregion("gfx1")->base()[pos];
	}
}

INPUT_CHANGED_MEMBER( x07_state::kb_func_keys )
{
	UINT8 data = 0;
	UINT8 idx = (UINT8)(FPTR)param;

	if (m_kb_on && newval)
	{
		UINT8 shift = (ioport("A1")->read() & 0x01);
		UINT16 udk_s = udk_offset[(shift*6) +  idx - 1];

		/* First 3 chars are used for description */
		udk_s += 3;

		do
		{
			data = m_t6834_ram[udk_s++];

			if (m_kb_size < 0xff && data != 0)
				m_t6834_ram[0x400 + m_kb_size++] = data;
		} while(data != 0);

		kb_irq();
	}
}

INPUT_CHANGED_MEMBER( x07_state::kb_keys )
{
	UINT8 modifier;
	UINT8 a1 = ioport("A1")->read();
	UINT8 bz = ioport("BZ")->read();
	UINT8 keycode = (UINT8)(FPTR)param;

	if (m_kb_on && !newval)
	{
		if (a1 == 0x01 && bz == 0x00)           //Shift
			modifier = 1;
		else if (a1 == 0x02 && bz == 0x00)      //CTRL
			modifier = 2;
		else if (a1 == 0x00 && bz == 0x08)      //Num
			modifier = 3;
		else if (a1 == 0x00 && bz == 0x02)      //Kana
			modifier = 4;
		else if (a1 == 0x01 && bz == 0x02)      //Shift+Kana
			modifier = 5;
		else if (a1 == 0x00 && bz == 0x04)      //Graph
			modifier = 6;
		else
			modifier = 0;

		if (m_kb_size < 0xff)
		{
			UINT8 idx = kb_get_index(keycode);
			m_t6834_ram[0x400 + m_kb_size++] = x07_keycodes[idx].codes[modifier];
		}

		kb_irq();
	}
}

INPUT_CHANGED_MEMBER( x07_state::kb_update_udk )
{
	draw_udk();
}

INPUT_CHANGED_MEMBER( x07_state::kb_break )
{
	if (newval)
	{
		if (!m_lcd_on)
		{
			m_lcd_on = 1;
			m_maincpu->set_state_int(Z80_PC, 0xc3c3);
		}
		else
		{
			m_regs_r[0] = 0x80;
			m_regs_r[1] = 0x05;
			m_regs_r[2] |= 0x01;
			m_maincpu->set_input_line(NSC800_RSTA, ASSERT_LINE );
			m_rsta_clear->adjust(attotime::from_msec(50));
		}
	}
}


void x07_state::kb_irq()
{
	if (m_kb_size)
	{
		m_regs_r[0] = 0;
		m_regs_r[1] = m_t6834_ram[0x400];
		memcpy(m_t6834_ram + 0x400, m_t6834_ram + 0x401, 0xff);
		m_kb_size--;
		m_regs_r[2] |= 0x01;
		m_maincpu->set_input_line(NSC800_RSTA, ASSERT_LINE);
		m_rsta_clear->adjust(attotime::from_msec(50));
	}
}


/***************************************************************************
    Video
***************************************************************************/

inline void x07_state::draw_char(UINT8 x, UINT8 y, UINT8 char_pos)
{
	if(x < 20 && y < 4)
		for(int cy = 0; cy < 8; cy++)
			for(int cx = 0; cx < 6; cx++)
				m_lcd_map[y * 8 + cy][x * 6 + cx] = (get_char(((char_pos << 3) + cy) & 0x7ff) & (0x80>>cx)) ? 1 : 0;
}


inline void x07_state::draw_point(UINT8 x, UINT8 y, UINT8 color)
{
	if(x < 120 && y < 32)
		m_lcd_map[y][x] = color;
}


inline void x07_state::draw_udk()
{
	UINT8 i, x, j;

	if (m_draw_udk)
		for(i = 0, x = 0; i < 5; i++)
		{
			UINT16 ofs = udk_offset[i + ((ioport("A1")->read()&0x01) ? 6 : 0)];
			draw_char(x++, 3, 0x83);
			for(j = 0; j < 3; j++)
				draw_char(x++, 3, m_t6834_ram[ofs++]);
		}
}

DEVICE_IMAGE_LOAD_MEMBER( x07_state, x07_card )
{
	UINT32 size = m_card->common_get_size("rom");

	// check card type
	if (image.software_entry() != nullptr)
	{
		const char *card_type = image.get_feature("card_type");

		if (strcmp(card_type, "xp140"))
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported card type");
			return IMAGE_INIT_FAIL;
		}
	}

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");

	m_card->ram_alloc(0x1000);

	return IMAGE_INIT_PASS;
}

PALETTE_INIT_MEMBER(x07_state, x07)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


UINT32 x07_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0);

	if (m_lcd_on)
	{
		for(int py = 0; py < 4; py++)
			for(int px = 0; px < 20; px++)
				for(int y = 0; y < 8; y++)
					for (int x=0; x<6; x++)
						if(m_cursor.on && m_blink && m_cursor.x == px && m_cursor.y == py)
							bitmap.pix16(py * 8 + y, px * 6 + x) = (y == 7) ? 1: 0;
						else
							bitmap.pix16(py * 8 + y, px * 6 + x) = m_lcd_map[py * 8 + y][px * 6 + x]? 1: 0;

	}

	return 0;
}


/***************************************************************************
    Machine
***************************************************************************/

READ8_MEMBER( x07_state::x07_io_r )
{
	UINT8 data = 0xff;

	switch(offset)
	{
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
		data = ((offset & 0x0f) < 8) ? get_char((m_font_code << 3) | (offset & 7)) : 0;
		break;

	case 0x90:
		data = 0x00;
		break;
	case 0xf6:
		if (m_cass_motor)   m_regs_r[6] |= 4;
		//fall through
	case 0xf0:
	case 0xf1:
	case 0xf3:
	case 0xf4:
	case 0xf5:
	case 0xf7:
		data = m_regs_r[offset & 7];
		break;

	case 0xf2:
		if(m_regs_w[5] & 4)
			m_regs_r[2] |= 2;
		else
			m_regs_r[2] &= 0xfd;
		data = m_regs_r[2] | 2;
		break;
	}

	return data;
}


WRITE8_MEMBER( x07_state::x07_io_w )
{
	switch(offset)
	{
	case 0x80:
		m_font_code = data;
		break;

	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
	case 0xf6:
	case 0xf7:
		m_regs_w[offset & 7] = data;
		break;

	case 0xf4:
		m_regs_r[4] = m_regs_w[4] = data;
		m_cass_motor = ((data & 0x0d) == 0x09) ? 1 : 0;

		if (m_cass_motor)
		{
			m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
			m_cass_poll->adjust(attotime::from_hz(48000), 0, attotime::from_hz(48000));
		}
		else
		{
			m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
			m_cass_poll->reset();
			m_cass_tick->reset();
		}

#if(1)
		if((data & 0x0e) == 0x0e)
		{
			m_beep->set_state(1);
			m_beep->set_frequency(192000 / ((m_regs_w[2] | (m_regs_w[3] << 8)) & 0x0fff));

			m_beep_stop->adjust(attotime::from_msec(m_ram->pointer()[0x450] * 0x20));
		}
		else
			m_beep->set_state(0);
#endif
		break;

	case 0xf5:
		if(data & 0x01)
			t6834_r();
		if(data & 0x02)
			t6834_w();
		if(data & 0x04)
			cassette_r();
		if(data & 0x08)
			cassette_w();
		if(data & 0x20)
			printer_w();

		m_regs_w[5] = data;
		break;
	}
}

static ADDRESS_MAP_START(x07_mem, AS_PROGRAM, 8, x07_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x0000, 0x1fff) AM_NOP     //RAM installed at runtime
	AM_RANGE(0x2000, 0x7fff) AM_NOP     //Memory Card RAM/ROM
	AM_RANGE(0x8000, 0x97ff) AM_RAM     //TV VRAM
	AM_RANGE(0x9800, 0x9fff) AM_UNMAP   //unused/unknown
	AM_RANGE(0xa000, 0xafff) AM_ROM     AM_REGION("x720", 0)        //TV ROM
	AM_RANGE(0xb000, 0xffff) AM_ROM     AM_REGION("basic", 0)       //BASIC ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( x07_io , AS_IO, 8, x07_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK (0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(x07_io_r, x07_io_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( x07 )
	PORT_START("BATTERY")
		PORT_CONFNAME( 0x40, 0x30, "Battery Status" )
		PORT_CONFSETTING( 0x30, DEF_STR( Normal ) )
		PORT_CONFSETTING( 0x40, "Low Battery" )
	PORT_START("CARDBATTERY")
		PORT_CONFNAME( 0x10, 0x00, "Card Battery Status" )
		PORT_CONFSETTING( 0x00, DEF_STR( Normal ) )
		PORT_CONFSETTING( 0x10, "Low Battery" )

	PORT_START("S1")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS")     PORT_CODE(KEYCODE_INSERT)           PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x12)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL")     PORT_CODE(KEYCODE_DEL)              PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x16)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT")   PORT_CODE(KEYCODE_RIGHT)            PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1c)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT")    PORT_CODE(KEYCODE_LEFT)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1d)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP")      PORT_CODE(KEYCODE_UP)               PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1e)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN")    PORT_CODE(KEYCODE_DOWN)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1f)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPC")     PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x20)
	PORT_START("S2")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x5a)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x58)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x43)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x56)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x42)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4e)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4d)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2c)
	PORT_START("S3")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x41)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x53)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x44)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x46)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x47)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x48)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4a)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4b)
	PORT_START("S4")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x51)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x57)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x45)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x52)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x54)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x59)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x55)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x49)
	PORT_START("S5")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x31)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x32)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x33)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x34)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x35)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x36)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')      PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x37)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x38)
	PORT_START("S6")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 1)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 2)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 3)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 4)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 5)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 6)
	PORT_START("S7")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2e)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2f)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR('?')                   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3f)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)  PORT_CHAR(13)  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x0d)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4f)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x50)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR('@') PORT_CHAR('\'')  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x40)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x5b)
	PORT_START("S8")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4c)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(';') PORT_CHAR('+')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3b)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3a)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x5d)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x39)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('|')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x30)
		PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2d)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('`')  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3d)
	PORT_START("BZ")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME")    PORT_CODE(KEYCODE_HOME)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x0b)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KANA")    PORT_CODE(KEYCODE_RALT)
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GRPH")    PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NUM")     PORT_CODE(KEYCODE_LALT)
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("OFF")     PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ON/BREAK") PORT_CODE(KEYCODE_F10)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_break, 0)
	PORT_START("A1")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)             PORT_CHAR(UCHAR_SHIFT_1)    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_update_udk, 0)
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
INPUT_PORTS_END


void x07_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	memcpy(data, memregion("default")->base(), size);
	m_warm_start = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(x07_state::blink_timer)
{
	m_blink = !m_blink;
}

TIMER_CALLBACK_MEMBER(x07_state::rsta_clear)
{
	m_maincpu->set_input_line(NSC800_RSTA, CLEAR_LINE);

	if (m_kb_size)
		kb_irq();
}

TIMER_CALLBACK_MEMBER(x07_state::rstb_clear)
{
	m_maincpu->set_input_line(NSC800_RSTB, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(x07_state::beep_stop)
{
	m_beep->set_state(0);
}

static const gfx_layout x07_charlayout =
{
	6, 8,                   /* 6 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 0, 1, 2, 3, 4, 5},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( x07 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, x07_charlayout, 0, 1 )
GFXDECODE_END

void x07_state::machine_start()
{
	UINT32 ram_size = m_ram->size();
	m_rsta_clear = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(x07_state::rsta_clear),this));
	m_rstb_clear = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(x07_state::rstb_clear),this));
	m_beep_stop = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(x07_state::beep_stop),this));
	m_cass_poll = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(x07_state::cassette_poll),this));
	m_cass_tick = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(x07_state::cassette_tick),this));

	m_nvram1->set_base(&m_t6834_ram, 0x800);
	m_nvram2->set_base(m_ram->pointer(), ram_size);

	/* Save State */
	save_item(NAME(m_sleep));
	save_item(NAME(m_warm_start));
	save_item(NAME(m_udk_on));
	save_item(NAME(m_draw_udk));
	save_item(NAME(m_sp_on));
	save_item(NAME(m_font_code));
	save_item(NAME(m_lcd_on));
	save_item(NAME(m_scroll_min));
	save_item(NAME(m_scroll_max));
	save_item(NAME(m_blink));
	save_item(NAME(m_kb_on));
	save_item(NAME(m_repeat_key));
	save_item(NAME(m_kb_size));
	save_item(NAME(m_prn_sendbit));
	save_item(NAME(m_prn_char_code));
	save_item(NAME(m_prn_size));
	save_item(NAME(m_cass_motor));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cass_clk));
	save_item(NAME(m_cass_state));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_t6834_ram));
	save_item(NAME(m_regs_r));
	save_item(NAME(m_regs_w));
	save_item(NAME(m_alarm));
	save_item(NAME(m_lcd_map));
	save_item(NAME(m_prn_buffer));
	save_item(NAME(m_in.read));
	save_item(NAME(m_in.write));
	save_item(NAME(m_in.data));
	save_item(NAME(m_out.read));
	save_item(NAME(m_out.write));
	save_item(NAME(m_out.data));
	save_item(NAME(m_locate.x));
	save_item(NAME(m_locate.y));
	save_item(NAME(m_locate.on));
	save_item(NAME(m_cursor.x));
	save_item(NAME(m_cursor.y));
	save_item(NAME(m_cursor.on));

	// install RAM
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_ram(0x0000, ram_size - 1, m_ram->pointer());

	// card
	if (m_card->exists())
	{
		// 0x4000 - 0x4fff   4KB RAM
		// 0x6000 - 0x7fff   8KB ROM
		program.install_read_handler(ram_size, ram_size + 0xfff, read8_delegate(FUNC(generic_slot_device::read_ram),(generic_slot_device*)m_card));
		program.install_write_handler(ram_size, ram_size + 0xfff, write8_delegate(FUNC(generic_slot_device::write_ram),(generic_slot_device*)m_card));
		program.install_read_handler(0x6000, 0x7fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_card));

		m_card->save_ram();
	}
}

void x07_state::machine_reset()
{
	memset(m_regs_r, 0, sizeof(m_regs_r));
	memset(m_regs_w, 0, sizeof(m_regs_w));
	memset(m_alarm, 0, sizeof(m_alarm));
	memset(&m_in, 0, sizeof(m_in));
	memset(&m_out, 0, sizeof(m_out));
	memset(&m_locate, 0, sizeof(m_locate));
	memset(&m_cursor, 0, sizeof(m_cursor));
	memset(m_prn_buffer, 0, sizeof(m_prn_buffer));
	memset(m_lcd_map, 0, sizeof(m_lcd_map));

	m_sleep = 0;
	m_udk_on = 0;
	m_draw_udk = 0;
	m_sp_on = 0;
	m_font_code = 0;
	m_lcd_on = 1;
	m_scroll_min = 0;
	m_scroll_max = 3;
	m_blink = 0;
	m_kb_on = 0;
	m_repeat_key = 0;
	m_kb_size = 0;
	m_repeat_key = 0;
	m_prn_sendbit = 0;
	m_prn_char_code = 0;
	m_prn_size = 0;

	m_regs_r[2] = ioport("CARDBATTERY")->read();

	m_maincpu->set_state_int(Z80_PC, 0xc3c3);
}

static MACHINE_CONFIG_START( x07, x07_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, XTAL_15_36MHz / 4)
	MCFG_CPU_PROGRAM_MAP(x07_mem)
	MCFG_CPU_IO_MAP(x07_io)

	/* video hardware */
	MCFG_SCREEN_ADD("lcd", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(x07_state, screen_update)
	MCFG_SCREEN_SIZE(120, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 120-1, 0, 32-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(x07_state, x07)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", x07)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 0.50 )
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* printer */
	MCFG_DEVICE_ADD("printer", PRINTER, 0)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("blink_timer", x07_state, blink_timer, attotime::from_msec(300))

	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram1", x07_state, nvram_init)   // t6834 RAM
	MCFG_NVRAM_ADD_0FILL("nvram2") // RAM banks

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	// 8KB  no expansion
	// 12KB XM-100
	// 16KB XR-100 or XM-101
	// 20KB XR-100 and XM-100
	// 24KB XR-100 and XM-101
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("8K,12K,20K,24k")

	/* Memory Card */
	MCFG_GENERIC_CARTSLOT_ADD("cardslot", generic_romram_plain_slot, "x07_card")
	MCFG_GENERIC_EXTENSIONS("rom,bin")
	MCFG_GENERIC_LOAD(x07_state, x07_card)

	/* cassette */
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(x07_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
	MCFG_CASSETTE_INTERFACE("x07_cass")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("card_list", "x07_card")
	MCFG_SOFTWARE_LIST_ADD("cass_list", "x07_cass")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( x07 )
	ROM_REGION( 0x6000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "x07.bin",  0x0000, 0x5001, BAD_DUMP CRC(61a6e3cc) SHA1(c53c22d33085ac7d5e490c5d8f41207729e5f08a) )       //very strange size...

	ROM_REGION( 0x1000, "x720", ROMREGION_ERASEFF )
	ROM_LOAD( "x720.bin", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x0800, BAD_DUMP CRC(b1e59a6e) SHA1(b0c06315a2d5c940a8f288fb6a3428d738696e69) )

	ROM_REGION( 0x0800, "default", ROMREGION_ERASE00 )
ROM_END

DRIVER_INIT_MEMBER(x07_state, x07)
{
	UINT8 *RAM = memregion("default")->base();
	UINT8 *GFX = memregion("gfx1")->base();

	for (int i = 0; i < 12; i++)
		strcpy((char *)RAM + udk_offset[i], udk_ini[i]);

	//copy default chars in the UDC
	memcpy(RAM + 0x200, GFX + 0x400, 0x100);
	memcpy(RAM + 0x300, GFX + 0x700, 0x100);
}


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT                COMPANY   FULLNAME    FLAGS */
COMP( 1983, x07,    0,      0,       x07,       x07,     x07_state,   x07,   "Canon",  "X-07",     MACHINE_SUPPORTS_SAVE)
