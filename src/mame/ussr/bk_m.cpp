// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

        BK machine driver by Miodrag Milanovic

        10/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "bk.h"


void bk_state::machine_start()
{
	save_item(NAME(m_scroll));
	save_item(NAME(m_sel1));
	save_item(NAME(m_misc));
	save_item(NAME(m_monitor));
	save_item(NAME(m_video_page));
	save_item(NAME(m_stop_disabled));

	m_tmpbmp.allocate(512, 256);
}

void bk_state::machine_reset()
{
	m_sel1 = SEL1_KEYDOWN | SEL1_MOTOR;
	m_scroll = 01330;
	m_monitor = BIT(m_config->read(), 0);
	m_misc = 0100;
	m_video_page = 0;
	m_stop_disabled = 0;
}

void bk_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_kbd->reset();
		m_timer->init_w();
		m_qbus->init_w();
	}
}

uint16_t bk_state::vid_scroll_r()
{
	return m_scroll;
}

// SEL1 register (0010 and 0010.01)
//
// 15-8 R   high byte of cpu start address
// 7    R   bitbanger cts in
// 7    W   cassette motor control, 1: off 0: on
// 6    R   keyboard any key down, 1: no 0: yes
// 6    W   cassette data and speaker out
// 5    R   cassette data in
// 5    W   cassette data and bitbanger rts out
// 4    R   bitbanger rx
// 4    W   bitbanger tx
// 2    R   updated
//
// only original 0010 has bitbanger wired to UP connector

uint16_t bk_state::sel1_r()
{
	double level = m_cassette->input();
	uint16_t data = 0100000 | m_sel1 | ((level > 0) ? SEL1_RX_CAS : 0);
	if (!machine().side_effects_disabled())
		m_sel1 &= ~SEL1_UPDATED;

	return data;
}

uint16_t bk_state::trap_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
	return ~0;
}

uint16_t bk_state::bk11_sel1_r()
{
	double level = m_cassette->input();
	uint16_t data = 0140200 | m_sel1 | ((level > 0) ? SEL1_RX_CAS : 0);
	if (!machine().side_effects_disabled())
		m_sel1 &= ~SEL1_UPDATED;

	return data;
}

void bk_state::vid_scroll_w(uint16_t data)
{
	m_scroll = data & 01377;
}

void bk_state::sel1_w(uint16_t data)
{
	m_sel1 |= SEL1_UPDATED;
	m_dac->write(BIT(data, 6));
	m_cassette->output(BIT(data, 6) ? 1.0 : -1.0);
	m_cassette->change_state((BIT(data, 7)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

void bk_state::trap_w(uint16_t data)
{
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
}

void bk_state::bk11_sel1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask != 0177777) return;
	if (!BIT(data, 11)) return;

	m_video_page = BIT(data, 2);
	m_dac->write(BIT(data, 6));
	m_cassette->output(BIT(data, 6) ? 1.0 : -1.0);
	m_cassette->change_state(
		(BIT(data, 7)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	bk11m_sel1_w(offset, data, mem_mask);
}

void bk_state::bk11m_sel1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (mem_mask != 0177777) return;

	m_sel1 |= SEL1_UPDATED;

	if (BIT(data, 11))
	{
		m_view1.select((data >> 12) & 7);
		switch (data & 033)
		{
			case 000: m_view2.select((data >> 8) & 7); break;
			case 001: m_view2.select(8); break;
			case 002: m_view2.select(9); break;
			case 010: m_view2.select(10); break;
			case 020: m_view2.select(11); break;
		}
	}
	else
	{
		m_stop_disabled = BIT(data, 12);
		m_dac->write(BIT(data, 6));
		m_cassette->output(BIT(data, 6) ? 1.0 : -1.0);
		m_cassette->change_state(
			(BIT(data, 7)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	}
}

QUICKLOAD_LOAD_MEMBER(bk_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t args[2];
	uint8_t ch = 0;

	image.fseek(0, SEEK_SET);

	if (image.fread(args, sizeof(args)) != sizeof(args))
	{
		return std::make_pair(image_error::UNSPECIFIED, "Unexpected EOF while getting file size");
	}

	const uint16_t quick_addr = little_endianize_int16(args[0]);
	const uint16_t quick_length = little_endianize_int16(args[1]);
	const uint16_t quick_end = quick_addr + quick_length - 1;

	if (quick_end > 077777)
	{
		return std::make_pair(image_error::INVALIDLENGTH, "File too large");
	}

	for (int i = 0; i < quick_length; i++)
	{
		unsigned j = (quick_addr + i);
		if (image.fread(&ch, 1) != 1)
		{
			return std::make_pair(image_error::UNSPECIFIED, util::string_format("Unexpected EOF while writing byte to %06o", j));
		}
		space.write_byte(j, ch);
	}

	space.write_word(0264, little_endianize_int16(quick_addr));
	space.write_word(0266, little_endianize_int16(quick_length));

	image.message("loaded, start %06o size %06o end %06o\n", quick_addr, quick_length, quick_end);

	return std::make_pair(std::error_condition(), std::string());
}

void bk_state::bk0010_palette(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 0, 255));
	palette.set_pen_color(2, rgb_t(0, 255, 0));
	palette.set_pen_color(3, rgb_t(255, 0, 0));
	palette.set_pen_color(4, rgb_t(255, 255, 255));
}

void bk_state::bk0011_palette(palette_device &palette)
{
	rgb_t const values[][3] =
	{
		{ rgb_t(0, 0, 255),     rgb_t(0, 255, 0),     rgb_t(255, 0, 0)     },	// 0
		{ rgb_t(255, 255, 0),   rgb_t(255, 0, 255),   rgb_t(255, 0, 0)     },	// 1
		{ rgb_t(0, 255, 255),   rgb_t(0, 0, 255),     rgb_t(255, 0, 255)   },	// 2
		{ rgb_t(0, 255, 0),     rgb_t(0, 255, 255),   rgb_t(255, 255, 0)   },	// 3
		{ rgb_t(255, 0, 255),   rgb_t(0, 255, 255),   rgb_t(255, 255, 255) },	// 4
		{ rgb_t(255, 255, 255), rgb_t(255, 255, 255), rgb_t(255, 255, 255) },	// 5
		{ rgb_t(192, 0, 0),     rgb_t(142, 0, 0),     rgb_t(255, 0, 0)     },	// 6
		{ rgb_t(192, 255, 0),   rgb_t(142, 255, 0),   rgb_t(255, 255, 0)   },	// 7
		{ rgb_t(192, 0, 255),   rgb_t(142, 0, 255),   rgb_t(255, 0, 255)   },	// 8
		{ rgb_t(142, 255, 0),   rgb_t(142, 0, 255),   rgb_t(142, 0, 0)     },	// 9
		{ rgb_t(192, 255, 0),   rgb_t(192, 0, 255),   rgb_t(192, 0, 0)     },	// 10
		{ rgb_t(0, 255, 255),   rgb_t(255, 255, 0),   rgb_t(255, 0, 0)     },	// 11
		{ rgb_t(255, 0, 0),     rgb_t(0, 255, 0),     rgb_t(0, 255, 255)   },	// 12
		{ rgb_t(0, 255, 255),   rgb_t(255, 255, 0),   rgb_t(255, 255, 255) },	// 13
		{ rgb_t(255, 255, 0),   rgb_t(0, 255, 0),     rgb_t(255, 255, 255) },	// 14
		{ rgb_t(0, 255, 255),   rgb_t(0, 255, 0),     rgb_t(255, 255, 255) }	// 15
	};
	for (int i = 0, j = 0; i < 16; i++)
	{
		palette.set_pen_color(j++, rgb_t(0, 0, 0));
		palette.set_pen_color(j++, values[i][0]);
		palette.set_pen_color(j++, values[i][1]);
		palette.set_pen_color(j++, values[i][2]);
	}
}

void bk_state::update_monitor(int state)
{
	m_monitor = state;
}

u32 bk_state::screen_update_bk10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 const mini = !BIT(m_scroll, 9);
	u16 const nOfs = (m_scroll & 255) + (mini ? 40 : -216);

	for (u16 y = 0; y < 256; y++)
	{
		for (u16 x = 0; x < 32; x++)
		{
			u16 const code = (y > 63 && mini) ? 0 : m_ram[0][((y + nOfs) % 256) * 32 + x];
			if (m_monitor)
			{
				for (u8 b = 0; b < 16; b += 2)
				{
					int pixel = (code >> b) & 3;
					bitmap.pix(y, x * 16 + b) = pixel;
					bitmap.pix(y, x * 16 + b + 1) = pixel;
				}
			}
			else
			{
				for (u8 b = 0; b < 16; b++)
					bitmap.pix(y, x * 16 + b) = BIT(code, b) << 2;
			}
		}
	}
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(bk_state::scanline_callback_bk11)
{
	u16 const mini = !BIT(m_scroll, 9);
	u16 const nOfs = (m_scroll & 255) + (mini ? 40 : -216);
	u16 const y = param;
	const pen_t *pen = m_palette->pens();

	for (u16 x = 0; x < 32; x++)
	{
		u16 const code = (y > 63 && mini) ? 0 : m_ram[5 + m_video_page][((y + nOfs) % 256) * 32 + x];
		if (m_monitor)
		{
			for (u8 b = 0; b < 16; b += 2)
			{
				int pixel = ((m_misc & 15) << 2) + ((code >> b) & 3);
				m_tmpbmp.pix(y, x * 16 + b) = pen[pixel];
				m_tmpbmp.pix(y, x * 16 + b + 1) = pen[pixel];
			}
		}
		else
		{
			for (u8 b = 0; b < 16; b++)
				m_tmpbmp.pix(y, x * 16 + b) = BIT(code, b) ? pen[63] : pen[0];
		}
	}
}

u32 bk_state::screen_update_bk11(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmpbmp, 0, 0, 0, 0, cliprect);
	return 0;
}
