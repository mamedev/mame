// license:GPL-2.0+
// copyright-holders:Curt Coder
/***************************************************************************

    Epson PX-8 HD6303 slave CPU and uPD7508 sub CPU HLE

    Based on the PX-8 OS Reference Manual chapters 11, 13 and 15, and
    the PX-8 Technical Manual chapters 2 and 7.

***************************************************************************/

#include "emu.h"
#include "px8.h"

#include "coreutil.h"

#define LOG_SUB     (1U << 1)
#define LOG_SLAVE   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

enum : uint8_t
{
	RCD00 = 0,
	RCD02 = 2,
	RCD04 = 11,
	RCD05 = 12,
	RCD06 = 13,
	RCD07 = 41,
	RCD12 = 61,
	RCD13 = 62
};

enum : uint8_t
{
	SUB_STATUS_SECOND = 0x20,
	SUB_STATUS_RESET = 0x10,
	SUB_STATUS_INITIALIZE = 0x08,
	SUB_STATUS_POWER_FAIL = 0x04,
	SUB_STATUS_ALARM = 0x02,
	SUB_STATUS_POWER_ON = 0x01
};

constexpr int GRAPHICS_WIDTH = 60;
constexpr int CHARACTER_WIDTH = 80;
constexpr int SCREEN_LINES = 64;

} // anonymous namespace

/***************************************************************************
    uPD7508 SUB CPU
***************************************************************************/

/*-------------------------------------------------
    sub_init - initialize sub CPU
-------------------------------------------------*/

void px8_state::sub_init()
{
	m_key_buf.clear();
	m_repeat_code = 0xff;
	m_repeat_start = 42;
	m_repeat_interval = 18;
	m_repeat_enb = true;
	m_key_intr_enb = true;
	m_second_intr_enb = false;
	m_alarm_intr_enb = false;
}

/*-------------------------------------------------
    sub_raise_interrupt - assert INT0
-------------------------------------------------*/

void px8_state::sub_raise_interrupt()
{
	m_isr |= INT0_7508;
	update_interrupt();
}

/*-------------------------------------------------
    sub_handshake - SIOR transfer after RESRDYSIO
-------------------------------------------------*/

void px8_state::sub_handshake()
{
	if (m_sub_rsp.empty())
		sub_command(m_sio);

	if (!m_sub_rsp.empty())
	{
		m_sio = m_sub_rsp.front();
		m_sub_rsp.pop_front();
	}

	m_rdysio = true;
}

/*-------------------------------------------------
    sub_command - sub CPU command/parameter byte
-------------------------------------------------*/

void px8_state::sub_command(uint8_t data)
{
	m_sub_cmd.push_back(data);

	static const std::pair<uint8_t, size_t> lengths[] =
	{
		{ 0x04, 2 }, { 0x14, 2 }, { 0x0b, 2 }, { 0x1b, 2 }, { 0x17, 9 }, { 0x19, 7 }
	};

	size_t length = 1;

	for (const auto &entry : lengths)
		if (entry.first == m_sub_cmd[0])
			length = entry.second;

	if (m_sub_cmd.size() < length)
		return;

	const std::vector<uint8_t> cmd = std::move(m_sub_cmd);
	m_sub_cmd.clear();

	LOGMASKED(LOG_SUB, "uPD7508 command %02x\n", cmd[0]);

	switch (cmd[0])
	{
	case 0x01:
		logerror("uPD7508 power off\n");
		break;

	case 0x02:
		m_isr &= ~INT0_7508;
		update_interrupt();

		if (m_sub_status)
		{
			m_sub_rsp.push_back(0xc0 | SUB_STATUS_POWER_ON | m_sub_status);
			m_sub_status = 0;
		}
		else if (!m_key_buf.empty())
		{
			m_sub_rsp.push_back(m_key_buf.front());
			m_key_buf.pop_front();
		}
		else
		{
			m_sub_rsp.push_back(0xbf);
		}

		if (m_sub_status || (m_key_intr_enb && !m_key_buf.empty()))
			sub_raise_interrupt();
		break;

	case 0x03:
		m_key_buf.clear();
		m_repeat_code = 0xff;
		m_repeat_start = 42;
		m_repeat_interval = 18;
		m_key_intr_enb = true;
		break;

	case 0x04:
		m_repeat_start = cmd[1] & 0x7f;
		break;

	case 0x14:
		m_repeat_interval = cmd[1] & 0x7f;
		break;

	case 0x24:
		m_sub_rsp.push_back(m_repeat_start);
		break;

	case 0x34:
		m_sub_rsp.push_back(m_repeat_interval);
		break;

	case 0x05:
		m_repeat_enb = false;
		break;

	case 0x15:
		m_repeat_enb = true;
		break;

	case 0x06:
		m_key_intr_enb = false;
		break;

	case 0x16:
		m_key_intr_enb = true;

		if (!m_key_buf.empty())
			sub_raise_interrupt();
		break;

	case 0x0d:
		m_second_intr_enb = false;
		break;

	case 0x1d:
		m_second_intr_enb = true;
		break;

	case 0x17:
		for (int i = 0; i < 7; i++)
		{
			uint8_t value = cmd[2 + i] & 0x7f;

			if (i == 0)
			{
				if ((cmd[1] & 0x7f) != 0x7f && value != 0x7f)
					m_rtc[0] = ((cmd[1] & 0x0f) * 10) + (value & 0x0f);
			}
			else if (value != 0x7f)
			{
				m_rtc[i] = (i == 6) ? (value & 0x07) : bcd_2_dec(value);
			}
		}
		break;

	case 0x07:
		m_sub_rsp.push_back(m_rtc[0] / 10);
		m_sub_rsp.push_back(m_rtc[0] % 10);

		for (int i = 1; i < 6; i++)
			m_sub_rsp.push_back(dec_2_bcd(m_rtc[i]));

		m_sub_rsp.push_back(m_rtc[6]);
		break;

	case 0x19:
		for (int i = 0; i < 6; i++)
			m_alarm[i] = cmd[1 + i] & 0x7f;
		break;

	case 0x09:
		m_sub_rsp.insert(m_sub_rsp.end(), std::begin(m_alarm), std::end(m_alarm));
		break;

	case 0x29:
		m_alarm_intr_enb = false;
		break;

	case 0x39:
		m_alarm_intr_enb = true;
		break;

	case 0x0c:
		m_sub_rsp.push_back(0xf0);
		break;

	case 0x1c:
		m_sub_rsp.push_back(0xb8);
		break;

	case 0x2c:
	case 0x3c:
		m_sub_rsp.push_back(0x00);
		break;

	case 0x0f:
		sub_init();
		break;

	case 0x0a:
		m_sub_rsp.push_back(m_sw4->read());
		break;

	case 0x0b:
		m_power_fail_voltage = cmd[1];
		break;

	case 0x1b:
		m_full_charge_voltage = cmd[1];
		break;

	case 0x08:
		m_sub_rsp.push_back(0x01);
		break;

	default:
		logerror("uPD7508 unknown command %02x\n", cmd[0]);
		break;
	}
}

/*-------------------------------------------------
    sub_key - place key code into key buffer
-------------------------------------------------*/

void px8_state::sub_key(uint8_t code)
{
	if (m_key_buf.size() >= 7)
		return;

	m_key_buf.push_back(code);

	if (m_key_intr_enb)
		sub_raise_interrupt();
}

/*-------------------------------------------------
    sub_keyboard_scan - keyboard matrix scan
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(px8_state::sub_keyboard_scan)
{
	for (int row = 0; row < 9; row++)
	{
		uint8_t data = m_ksc_io[row]->read();
		uint8_t changed = data ^ m_key_state[row];
		m_key_state[row] = data;

		for (int bit = 0; bit < 8; bit++)
		{
			if (!BIT(changed, bit))
				continue;

			bool pressed = !BIT(data, bit);

			if (row == 0 && bit >= 2)
			{
				sub_key((pressed ? 0xb0 : 0xa0) | bit);
				continue;
			}

			uint8_t code = (row == 0) ? (0x80 | bit) : (((row - 1) << 4) | bit);

			if (pressed)
			{
				sub_key(code);

				if (code > 0x07)
				{
					m_repeat_code = code;
					m_repeat_next = machine().time() + attotime::from_ticks(m_repeat_start, 64);
				}
			}
			else if (code == m_repeat_code)
			{
				m_repeat_code = 0xff;
			}
		}
	}

	if (m_repeat_enb && m_repeat_code != 0xff && machine().time() >= m_repeat_next)
	{
		sub_key(m_repeat_code);
		m_repeat_next = machine().time() + attotime::from_ticks(std::max<uint8_t>(m_repeat_interval, 1), 256);
	}
}

/*-------------------------------------------------
    sub_rtc_tick - advance calendar clock
-------------------------------------------------*/

void px8_state::sub_rtc_tick()
{
	static const uint8_t days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (++m_rtc[5] < 60)
		return;

	m_rtc[5] = 0;

	if (++m_rtc[4] < 60)
		return;

	m_rtc[4] = 0;

	if (++m_rtc[3] < 24)
		return;

	m_rtc[3] = 0;
	m_rtc[6] = (m_rtc[6] + 1) % 7;

	int month = std::clamp<int>(m_rtc[1], 1, 12);
	int days = days_in_month[month - 1] + ((month == 2 && !(m_rtc[0] % 4)) ? 1 : 0);

	if (++m_rtc[2] <= days)
		return;

	m_rtc[2] = 1;

	if (++m_rtc[1] <= 12)
		return;

	m_rtc[1] = 1;
	m_rtc[0] = (m_rtc[0] + 1) % 100;
}

/*-------------------------------------------------
    sub_second_tick - one second interval timer
-------------------------------------------------*/

TIMER_CALLBACK_MEMBER(px8_state::sub_second_tick)
{
	sub_rtc_tick();

	if (m_second_intr_enb)
	{
		m_sub_status |= SUB_STATUS_SECOND;
		sub_raise_interrupt();
	}
}

/***************************************************************************
    HD6303 SLAVE CPU
***************************************************************************/

/*-------------------------------------------------
    slave_status_r - SED1320 PSR read
-------------------------------------------------*/

uint8_t px8_state::slave_status_r()
{
	uint8_t data = 0;

	if (!m_slave_rsp.empty())
	{
		data |= 0x02;

		if (m_slave_rsp.front().second)
			data |= 0x08;
	}

	return data;
}

/*-------------------------------------------------
    slave_data_r - SED1320 PDOR read
-------------------------------------------------*/

uint8_t px8_state::slave_data_r()
{
	uint8_t data = 0xff;

	if (!m_slave_rsp.empty())
	{
		data = m_slave_rsp.front().first;
		m_slave_rsp.pop_front();
	}

	return data;
}

/*-------------------------------------------------
    slave_cmd_w - SED1320 PDIR command write
-------------------------------------------------*/

void px8_state::slave_cmd_w(uint8_t data)
{
	m_slave_cmd = data;
	m_slave_buf.clear();
	m_slave_rsp.clear();

	slave_execute();
}

/*-------------------------------------------------
    slave_data_w - SED1320 PDIR data write
-------------------------------------------------*/

void px8_state::slave_data_w(uint8_t data)
{
	m_slave_buf.push_back(data);

	slave_execute();
}

/*-------------------------------------------------
    slave_return - return code to main CPU
-------------------------------------------------*/

void px8_state::slave_return(uint8_t code)
{
	m_slave_rsp.emplace_back(code, true);
	m_slave_buf.clear();
}

/*-------------------------------------------------
    slave_data - return parameter to main CPU
-------------------------------------------------*/

void px8_state::slave_data(uint8_t data)
{
	m_slave_rsp.emplace_back(data, false);
}

/*-------------------------------------------------
    slave_glyph - character generator lookup
-------------------------------------------------*/

const uint8_t *px8_state::slave_glyph(uint8_t code)
{
	if (code >= m_udc_start)
		return &m_sram[0x8000 + ((code - m_udc_start) << 3)];

	return &m_font[code << 3];
}

/*-------------------------------------------------
    slave_get_point - read graphics screen dot
-------------------------------------------------*/

bool px8_state::slave_get_point(int x, int y)
{
	return BIT(m_sram[uint16_t(m_gs_addr + (y * GRAPHICS_WIDTH) + (x >> 3))], 7 - (x & 7));
}

/*-------------------------------------------------
    slave_set_point - modify graphics screen dot
-------------------------------------------------*/

void px8_state::slave_set_point(int x, int y, uint8_t op)
{
	if (x < 0 || x >= GRAPHICS_WIDTH * 8 || y < 0 || y >= SCREEN_LINES)
		return;

	uint8_t &data = m_sram[uint16_t(m_gs_addr + (y * GRAPHICS_WIDTH) + (x >> 3))];
	uint8_t mask = 0x80 >> (x & 7);

	switch (op)
	{
	case 1:
		data &= ~mask;
		break;

	case 2:
		data |= mask;
		break;

	case 3:
		data ^= mask;
		break;
	}
}

/*-------------------------------------------------
    slave_execute - slave CPU command processing
-------------------------------------------------*/

void px8_state::slave_execute()
{
	const auto &sp = m_slave_buf;
	const size_t n = sp.size();

	auto word = [&sp] (int i) { return uint16_t((sp[i] << 8) | sp[i + 1]); };
	auto count = [] (uint8_t value) { return value ? int(value) : 256; };

	auto operate = [] (uint8_t &data, uint8_t value, uint8_t op)
	{
		switch (op)
		{
		case 1:
			data &= value;
			break;

		case 2:
			data |= value;
			break;

		case 3:
			data ^= value;
			break;

		default:
			data = value;
			break;
		}
	};

	auto move_block = [this, &sp] (uint16_t base, int width)
	{
		int sx = sp[0], sy = sp[1], w = sp[2], h = sp[3], dx = sp[4], dy = sp[5];

		if (!w || !h || sx + w > width || sy + h > SCREEN_LINES || dx + w > width || dy + h > SCREEN_LINES)
		{
			slave_return(RCD04);
			return;
		}

		std::vector<uint8_t> block;

		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				block.push_back(m_sram[uint16_t(base + ((sy + y) * width) + sx + x)]);

		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				m_sram[uint16_t(base + ((dy + y) * width) + dx + x)] = block[(y * w) + x];

		slave_return(RCD00);
	};

	auto blocks_valid = [&sp] (int width, int max)
	{
		if (sp[0] > max)
			return false;

		for (int i = 0; i < sp[0]; i++)
			if (sp[1 + (i * 3)] >= width || sp[2 + (i * 3)] >= SCREEN_LINES)
				return false;

		return true;
	};

	switch (m_slave_cmd)
	{
	case 0x00:
		if (n == 2)
		{
			slave_return(RCD00);
			slave_data(m_sram[word(0)]);
		}
		break;

	case 0x01:
		if (n == 4)
		{
			operate(m_sram[word(0)], sp[2], sp[3]);
			slave_return(RCD00);
		}
		break;

	case 0x02:
		if (n == 2)
		{
			logerror("HD6303 execute routine at %04x not supported\n", word(0));
			slave_return(RCD00);
		}
		break;

	case 0x0a:
	case 0x0b:
		slave_return(RCD00);
		break;

	case 0x10:
		if (n == 16)
		{
			m_cs_addr = word(0);
			m_gs_addr = word(2);
			m_udc_start = sp[11];
			slave_return(RCD00);
		}
		break;

	case 0x11:
		if (n == 1)
		{
			m_lcd_on = sp[0] != 0;
			slave_return(RCD00);
		}
		break;

	case 0x12:
		if (n == 1)
		{
			m_char_mode = sp[0] != 0;
			m_scr_ptr = m_char_mode ? (m_cs_addr + (m_wnd_y * CHARACTER_WIDTH) + m_wnd_x) : m_gs_addr;
			slave_return(RCD00);
		}
		break;

	case 0x13:
		slave_return(RCD00);
		slave_data(0x80 | ((m_scr_ptr >> 8) & 0x1f));
		slave_data(m_scr_ptr & 0xff);
		break;

	case 0x14:
		if (n == 2)
		{
			m_scr_ptr = word(0);
			slave_return(RCD00);
		}
		break;

	case 0x15:
		if (n == 1)
		{
			m_seven_lines = sp[0] != 0;
			slave_return(RCD00);
		}
		break;

	case 0x16:
		if (n == 1)
		{
			m_curs_mode = sp[0] & 0x07;
			slave_return(RCD00);
		}
		break;

	case 0x17:
		slave_return(RCD00);
		slave_data(m_curs_x);
		slave_data(m_curs_y);
		break;

	case 0x18:
		if (n == 2)
		{
			m_curs_x = sp[0];
			m_curs_y = sp[1];
			slave_return(RCD00);
		}
		break;

	case 0x19:
		if (n == 1)
		{
			m_flash = sp[0];
			slave_return(RCD00);
		}
		break;

	case 0x1a:
		if (n == 4)
		{
			for (int line = sp[2]; line < sp[2] + count(sp[3]); line++)
			{
				if (sp[0] && line < SCREEN_LINES)
					std::fill_n(&m_sram[uint16_t(m_cs_addr + (line * CHARACTER_WIDTH))], CHARACTER_WIDTH, sp[1]);
				else if (!sp[0] && line < 8)
					std::fill_n(&m_sram[uint16_t(m_gs_addr + (line * GRAPHICS_WIDTH * 8))], GRAPHICS_WIDTH * 8, sp[1]);
			}

			slave_return(RCD00);
		}
		break;

	case 0x1b:
		if (n == 1)
		{
			const uint8_t *glyph = slave_glyph(sp[0]);

			slave_return(RCD00);

			for (int i = 0; i < 8; i++)
				slave_data(glyph[i]);
		}
		break;

	case 0x20:
		if (n >= 3 && n == size_t(3 + (sp[1] * sp[2])))
		{
			if (!sp[0])
			{
				m_gudc.clear();
				slave_return(RCD00);
			}
			else if (!sp[1] || !sp[2] || sp[1] * sp[2] > 255)
			{
				slave_return(RCD04);
			}
			else
			{
				m_gudc.emplace(sp[0], std::vector<uint8_t>(sp.begin() + 1, sp.end()));
				slave_return(RCD00);
			}
		}
		break;

	case 0x21:
		if (n >= 1 && n == size_t(1 + (sp[0] * 3)))
			slave_return(blocks_valid(GRAPHICS_WIDTH, 144) ? RCD00 : RCD04);
		break;

	case 0x22:
		if (n == 4)
		{
			const uint8_t *glyph = slave_glyph(sp[3]);
			int x = word(0);

			for (int l = 0; l < 8; l++)
				for (int i = 0; i < 6; i++)
					slave_set_point((x + i) % (GRAPHICS_WIDTH * 8), sp[2] + l, BIT(glyph[l], 5 - i) ? 2 : 1);

			slave_return(RCD00);
		}
		break;

	case 0x23:
		if (n == 3)
		{
			auto udc = m_gudc.find(sp[2]);

			if (udc == m_gudc.end())
			{
				slave_return(RCD05);
				break;
			}

			const std::vector<uint8_t> &def = udc->second;
			int w = def[0], h = def[1];

			for (int y = 0; y < h; y++)
				for (int x = 0; x < w; x++)
					if (sp[0] + x < GRAPHICS_WIDTH && sp[1] + y < SCREEN_LINES)
						m_sram[uint16_t(m_gs_addr + ((sp[1] + y) * GRAPHICS_WIDTH) + sp[0] + x)] = def[2 + (y * w) + x];

			slave_return(RCD00);
		}
		break;

	case 0x24:
		if (n == 3)
		{
			if (sp[0] >= GRAPHICS_WIDTH || sp[1] >= SCREEN_LINES)
			{
				slave_return(RCD04);
				break;
			}

			uint16_t addr = m_gs_addr + (sp[1] * GRAPHICS_WIDTH) + sp[0];

			slave_return(RCD00);

			for (int i = 0; i < count(sp[2]); i++)
				slave_data(m_sram[uint16_t(addr + i)]);
		}
		break;

	case 0x25:
		if (n >= 4 && n == size_t(5 + (sp[2] * sp[3])))
		{
			if (!sp[2] || !sp[3] || sp[2] * sp[3] > 255 || sp[0] >= GRAPHICS_WIDTH || sp[1] >= SCREEN_LINES)
			{
				slave_return(RCD04);
				break;
			}

			for (int y = 0; y < sp[3]; y++)
				for (int x = 0; x < sp[2]; x++)
					if (sp[0] + x < GRAPHICS_WIDTH && sp[1] + y < SCREEN_LINES)
						operate(m_sram[uint16_t(m_gs_addr + ((sp[1] + y) * GRAPHICS_WIDTH) + sp[0] + x)], sp[5 + (y * sp[2]) + x], sp[4]);

			slave_return(RCD00);
		}
		break;

	case 0x26:
		if (n == 6)
			move_block(m_gs_addr, GRAPHICS_WIDTH);
		break;

	case 0x27:
		if (n == 4)
		{
			int x = word(0);

			if (x >= GRAPHICS_WIDTH * 8 || sp[2] >= SCREEN_LINES)
			{
				slave_return(RCD04);
				break;
			}

			slave_set_point(x, sp[2], sp[3]);
			slave_return(RCD00);
		}
		break;

	case 0x28:
		if (n == 3)
		{
			int x = word(0);

			if (x >= GRAPHICS_WIDTH * 8 || sp[2] >= SCREEN_LINES)
			{
				slave_return(RCD04);
				break;
			}

			slave_return(RCD00);
			slave_data(slave_get_point(x, sp[2]) ? 1 : 0);
		}
		break;

	case 0x29:
		if (n == 11)
		{
			int x0 = int16_t(word(0)), y0 = int16_t(word(2));
			int x1 = int16_t(word(4)), y1 = int16_t(word(6));
			uint16_t vector = word(8);
			uint8_t mode = sp[10];

			int dx = std::abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
			int dy = -std::abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
			int error = dx + dy;

			while (true)
			{
				if (BIT(vector, 15))
					slave_set_point(x0, y0, mode);

				vector = (vector << 1) | BIT(vector, 15);

				if (x0 == x1 && y0 == y1)
					break;

				int e2 = error * 2;

				if (e2 >= dy)
				{
					error += dy;
					x0 += sx;
				}

				if (e2 <= dx)
				{
					error += dx;
					y0 += sy;
				}
			}

			slave_return(RCD00);
		}
		break;

	case 0x30:
		if (n == 9)
		{
			if (sp[0] < m_udc_start)
			{
				slave_return(RCD06);
				break;
			}

			std::copy_n(&sp[1], 8, &m_sram[0x8000 + ((sp[0] - m_udc_start) << 3)]);
			slave_return(RCD00);
		}
		break;

	case 0x31:
		if (n >= 1 && n == size_t(1 + (sp[0] * 3)))
			slave_return(blocks_valid(CHARACTER_WIDTH, 40) ? RCD00 : RCD04);
		break;

	case 0x32:
		slave_return(RCD00);
		slave_data(m_wnd_x);
		slave_data(m_wnd_y);
		break;

	case 0x33:
		if (n == 2)
		{
			m_wnd_x = sp[0];
			m_wnd_y = sp[1];

			if (m_char_mode)
				m_scr_ptr = m_cs_addr + (m_wnd_y * CHARACTER_WIDTH) + m_wnd_x;

			slave_return(RCD00);
		}
		break;

	case 0x34:
		if (n == 3)
		{
			uint16_t addr = m_cs_addr + (sp[1] * CHARACTER_WIDTH) + sp[0];

			slave_return(RCD00);

			for (int i = 0; i < count(sp[2]); i++)
				slave_data(m_sram[uint16_t(addr + i)]);
		}
		break;

	case 0x35:
		if (n >= 3 && n == size_t(3 + sp[2]))
		{
			int offset = (sp[1] * CHARACTER_WIDTH) + sp[0];

			for (int i = 0; i < sp[2] && offset + i < CHARACTER_WIDTH * SCREEN_LINES; i++)
				m_sram[uint16_t(m_cs_addr + offset + i)] = sp[3 + i];

			slave_return(RCD00);
		}
		break;

	case 0x36:
		if (n == 6)
			move_block(m_cs_addr, CHARACTER_WIDTH);
		break;

	case 0x40:
		slave_return(RCD00);
		slave_data(m_mct_protect ? 0x01 : 0x00);
		break;

	case 0x41:
	case 0x45:
	case 0x47:
		slave_return(RCD07);
		break;

	case 0x42:
	case 0x46:
	case 0x48:
	case 0x49:
	case 0x4a:
		slave_return(RCD00);
		break;

	case 0x43:
	case 0x44:
		if (n == 2)
			slave_return(RCD07);
		break;

	case 0x4b:
		slave_return(RCD00);
		slave_data(0x00);
		break;

	case 0x4c:
		slave_return(RCD00);
		slave_data(m_mct_counter >> 8);
		slave_data(m_mct_counter & 0xff);
		break;

	case 0x4d:
		if (n == 2)
		{
			m_mct_counter = word(0);
			slave_return(RCD00);
		}
		break;

	case 0x51:
	case 0x52:
		if (n >= 2 && n == size_t(3 + (word(0) ? word(0) : 0x10000)))
			slave_return(RCD07);
		break;

	case 0x53:
	case 0x54:
		if (n == 3)
			slave_return(RCD07);
		break;

	case 0x55:
		if (n == 2)
		{
			m_mct_protect = true;
			slave_return(RCD00);
		}
		break;

	case 0x56:
		m_mct_protect = false;
		slave_return(RCD00);
		break;

	case 0x60:
		slave_return(RCD00);
		slave_data(0x20);
		break;

	case 0x61:
	case 0x63:
		if (n == 1)
			slave_return(RCD00);
		break;

	case 0x62:
		slave_return(RCD13);
		break;

	case 0x64:
		if (n >= 6 && n == size_t(7 + sp[5]))
			slave_return(RCD12);
		break;

	case 0x65:
		slave_return(RCD12);
		break;

	case 0x70:
		if (n == 1)
		{
			m_prom_power = sp[0] != 0;
			slave_return(RCD00);
		}
		break;

	case 0x71:
		if (n == 4)
		{
			uint16_t addr = word(1);

			slave_return(RCD00);

			for (int i = 0; i < count(sp[3]); i++)
			{
				uint16_t logical = addr + i;
				generic_slot_device &capsule = *m_capsule[BIT(logical, 15)];
				uint32_t size = capsule.get_rom_size();
				uint16_t physical = (logical ^ 0x4000) & 0x7fff;

				slave_data(size ? capsule.read_rom(physical & (size - 1)) : 0xff);
			}

			if (!sp[0])
				m_prom_power = false;
		}
		break;

	case 0x72:
		if (n == 1)
			slave_return(RCD00);
		break;

	case 0x73:
	case 0x74:
		if (n == 3)
			slave_return(RCD00);
		break;

	default:
		logerror("HD6303 unknown command %02x\n", m_slave_cmd);
		slave_return(RCD02);
		break;
	}
}

/***************************************************************************
    SED1320 DISPLAY
***************************************************************************/

/*-------------------------------------------------
    lcdc_init - initialize LCD controller
-------------------------------------------------*/

void px8_state::lcdc_init()
{
	static const uint8_t system_set[] = { 0x30, 0x87, 0x07, 0x3b, 0x3b, 0x3f, 0x3c, 0x00 };
	static const uint8_t scroll[] = { 0x00, 0x00, 0x3f, 0x00, 0x10, 0x3f, 0x00, 0x00, 0x00, 0x00 };

	m_lcdc->command_w(0x40);

	for (uint8_t data : system_set)
		m_lcdc->data_w(data);

	m_lcdc->command_w(0x44);

	for (uint8_t data : scroll)
		m_lcdc->data_w(data);

	m_lcdc->command_w(0x5a);
	m_lcdc->data_w(0x00);

	m_lcdc->command_w(0x5b);
	m_lcdc->data_w(0x0c);

	m_lcdc->command_w(0x4c);

	m_lcdc->command_w(0x59);
	m_lcdc->data_w(0x04);

	m_lcd_on_shadow = true;
	std::fill(std::begin(m_lcd_shadow), std::end(m_lcd_shadow), 0);
}

/*-------------------------------------------------
    lcdc_update - render V-RAM to LCD controller
-------------------------------------------------*/

void px8_state::lcdc_update(int state)
{
	if (!state)
		return;

	uint8_t frame[GRAPHICS_WIDTH * SCREEN_LINES];
	std::fill(std::begin(frame), std::end(frame), 0);

	if (m_char_mode)
	{
		int rows = m_seven_lines ? 7 : 8;
		int pitch = m_seven_lines ? 9 : 8;
		bool cursor_visible = BIT(m_curs_mode, 0) && (!BIT(m_curs_mode, 1) || BIT(m_screen->frame_number(), 5));

		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < CHARACTER_WIDTH; column++)
			{
				uint16_t addr = 0x8000 | ((m_scr_ptr + (row * CHARACTER_WIDTH) + column) & 0x1fff);
				const uint8_t *glyph = slave_glyph(m_sram[addr]);
				bool cursor = cursor_visible && column == m_curs_x && row == m_curs_y;

				for (int line = 0; line < 8; line++)
				{
					uint8_t pattern = glyph[line] & 0x3f;

					if (cursor && (BIT(m_curs_mode, 2) || line == 7))
						pattern = 0x3f;

					for (int i = 0; i < 6; i++)
					{
						int x = (column * 6) + i;

						if (BIT(pattern, 5 - i))
							frame[((row * pitch) + line) * GRAPHICS_WIDTH + (x >> 3)] |= 0x80 >> (x & 7);
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < GRAPHICS_WIDTH * SCREEN_LINES; i++)
			frame[i] = m_sram[0x8000 | ((m_scr_ptr + i) & 0x1fff)];
	}

	if (m_lcd_on != m_lcd_on_shadow)
	{
		m_lcd_on_shadow = m_lcd_on;
		m_lcdc->command_w(m_lcd_on ? 0x59 : 0x58);
		m_lcdc->data_w(0x04);
	}

	int next = -1;

	for (int i = 0; i < GRAPHICS_WIDTH * SCREEN_LINES; i++)
	{
		if (frame[i] == m_lcd_shadow[i])
			continue;

		if (i != next)
		{
			m_lcdc->command_w(0x46);
			m_lcdc->data_w(i & 0xff);
			m_lcdc->data_w(i >> 8);
			m_lcdc->command_w(0x42);
		}

		m_lcdc->data_w(frame[i]);
		m_lcd_shadow[i] = frame[i];
		next = i + 1;
	}
}
