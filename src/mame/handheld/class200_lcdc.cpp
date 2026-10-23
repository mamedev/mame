// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "class200_lcdc.h"

// unknown LCDC, possibly a ST7775R (if so, expand this and turn it into a device in the devices folder)

DEFINE_DEVICE_TYPE(CLASS200_LCDC, class200_lcdc_device, "class200_lcdc", "unknown LCDC (class200)")

class200_lcdc_device::class200_lcdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CLASS200_LCDC, tag, owner, clock)
{
}

void class200_lcdc_device::render_to_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!(m_lcd_regs[0x07] & 0x0003))
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return;
	}
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			const u16 pixel = m_gram[x * 176 + y];
			const u8 r = (pixel >> 11) & 0x1f;
			const u8 g = (pixel >> 5) & 0x3f;
			const u8 b = pixel & 0x1f;
			// class200 PCB/panel uses BGR subpixel order; the LCD entry-mode
			// BGR bit still reverses the GRAM transfer order relative to it.
			// This is a provisional board-level colour calibration.
			const bool bgr = !BIT(m_lcd_regs[0x03], 12);
			bitmap.pix(y, x) = bgr
				? rgb_t((b << 3) | (b >> 2), (g << 2) | (g >> 4), (r << 3) | (r >> 2))
				: rgb_t((r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2));
		}
	}
}

void class200_lcdc_device::class200_lcdc_advance()
{
	const u16 mode = m_lcd_regs[0x03];
	const bool x_increment = bool(mode & 0x10);
	const bool y_increment = bool(mode & 0x20);
	const unsigned h_start = std::min<unsigned>(m_lcd_regs[0x37] & 0xff, 175);
	const unsigned h_end = std::min<unsigned>(m_lcd_regs[0x36] & 0xff, 175);
	const unsigned v_start = std::min<unsigned>(m_lcd_regs[0x39] & 0xff, 219);
	const unsigned v_end = std::min<unsigned>(m_lcd_regs[0x38] & 0xff, 219);
	if (h_start > h_end || v_start > v_end)
		return;

	const auto advance = [](u16 &coord, unsigned lo, unsigned hi, bool increase)
	{
		if (increase)
		{
			if (coord >= hi) { coord = lo; return true; }
			coord++;
		}
		else
		{
			if (coord <= lo) { coord = hi; return true; }
			coord--;
		}
		return false;
	};

	if (mode & 0x0008)
	{
		if (advance(m_lcd_regs[0x21], v_start, v_end, y_increment))
			advance(m_lcd_regs[0x20], h_start, h_end, x_increment);
	}
	else
	{
		if (advance(m_lcd_regs[0x20], h_start, h_end, x_increment))
			advance(m_lcd_regs[0x21], v_start, v_end, y_increment);
	}
}


void class200_lcdc_device::lcdc_command_w(u8 data)
{
	if (!m_lcd_command_phase)
	{
		m_lcd_command_word = u16(data) << 8;
		m_lcd_command_phase = 1;
	}
	else
	{
		m_lcd_command_word |= data;
		m_lcd_reg = m_lcd_command_word & 0xff;
		m_lcd_command_phase = 0;
		m_lcd_data_phase = m_lcd_read_phase = 0;
	}
}

void class200_lcdc_device::lcdc_data_w(u8 data)
{
	if (!m_lcd_data_phase)
	{
		m_lcd_data_word = u16(data) << 8;
		m_lcd_data_phase = 1;
		return;
	}

	m_lcd_data_word |= data;
	m_lcd_data_phase = 0;
	const u16 value = m_lcd_data_word;
	const u8 r = m_lcd_reg;
	if (r == 0x22)
	{
		++m_lcd_gram_writes;
		if (value != 0 && !m_lcd_first_nonblack_logged)
		{
			logerror("class200_lcdc: first non-black LCD pixel=%04X cursor=(%u,%u) writes=%u\n",
				value, m_lcd_regs[0x20] & 0xff, m_lcd_regs[0x21] & 0xff,
				m_lcd_gram_writes);
			m_lcd_first_nonblack_logged = true;
		}
		const unsigned col = m_lcd_regs[0x20] & 0xff;
		const unsigned row = m_lcd_regs[0x21] & 0xff;
		if (col < 176 && row < 220)
			m_gram[row * 176 + col] = value;
		class200_lcdc_advance();
	}
	else
	{
		m_lcd_regs[r] = value;

		if (r == 0x03)
			logerror("class200_lcdc: LCD R03 entry mode=%04X BGR=%d\n", value, BIT(value, 12));

		if (r == 0x07)
			logerror("class200_lcdc: display control=%04X, GRAM writes=%u\n", value, m_lcd_gram_writes);
	}
}

u8 class200_lcdc_device::lcdc_data_r()
{
	const u16 val = (m_lcd_reg == 0x22)
		? m_gram[(m_lcd_regs[0x21] % 220) * 176 + (m_lcd_regs[0x20] % 176)]
		: m_lcd_regs[m_lcd_reg];
	const u8 result = m_lcd_read_phase ? u8(val) : u8(val >> 8);
	m_lcd_read_phase ^= 1;
	return result;
}


void class200_lcdc_device::device_start()
{
	m_gram = make_unique_clear<u16 []>(176 * 220);

	save_pointer(NAME(m_gram), 176 * 220);
	save_item(NAME(m_lcd_regs));
	save_item(NAME(m_lcd_command_word));
	save_item(NAME(m_lcd_data_word));
	save_item(NAME(m_lcd_command_phase));
	save_item(NAME(m_lcd_data_phase));
	save_item(NAME(m_lcd_read_phase));
	save_item(NAME(m_lcd_reg));
	save_item(NAME(m_lcd_first_nonblack_logged));
	save_item(NAME(m_lcd_gram_writes));
}

void class200_lcdc_device::device_reset()
{
	for (int i = 0; i < 176 * 220; i++)
		m_gram[i] = 0;

	std::fill(std::begin(m_lcd_regs), std::end(m_lcd_regs), 0);
	m_lcd_regs[0x36] = 0x00af; // H end, 176 columns
	m_lcd_regs[0x38] = 0x00db; // V end, 220 rows
	m_lcd_regs[0x03] = 0x0030;
	m_lcd_command_word = m_lcd_data_word = 0;
	m_lcd_command_phase = m_lcd_data_phase = 0;
	m_lcd_read_phase = m_lcd_reg = 0;
	m_lcd_first_nonblack_logged = false;
	m_lcd_gram_writes = 0;
}
