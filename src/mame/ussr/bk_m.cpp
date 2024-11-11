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
}

void bk_state::machine_reset()
{
	m_sel1 = SEL1_KEYDOWN | SEL1_MOTOR;
	m_scroll = 01330;
	m_monitor = ioport("CONFIG")->read();
}

void bk_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_kbd->reset();
		m_qbus->init_w();
	}
}

uint16_t bk_state::vid_scroll_r()
{
	return m_scroll;
}

// SEL1 register (0010 and 0010.01)
//
// 15-8	R	high byte of cpu start address
// 7	R	bitbanger cts in
// 7	W	cassette motor control, 1: off 0: on
// 6	R	keyboard any key down, 1: no 0: yes
// 6	W	cassette data and speaker out
// 5	R	cassette data in
// 5	W	cassette data and bitbanger rts out
// 4	R	bitbanger rx
// 4	W	bitbanger tx
// 2	R	updated
//
// only original 0010 has bitbanger wired to UP connector

uint16_t bk_state::sel1_r()
{
	double level = m_cassette->input();
	uint16_t data = 0100000 | m_sel1 | ((level < 0) ? 0 : SEL1_RX_CAS);
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

QUICKLOAD_LOAD_MEMBER(bk_state::quickload_cb)
{
	if (image.length() > 0100000)
		return std::make_pair(image_error::INVALIDLENGTH, "File too long (must be no larger than 32 KB)");

	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t addr, size, offset;

	image.fread(&addr, 2);
	addr = little_endianize_int16(addr);
	image.fread(&size, 2);
	size = little_endianize_int16(size);
	logerror("bk bin load: addr %06o size %06o\n", addr, size);

	std::vector<uint16_t> buffer((size+1)/2);
	image.fread(&buffer[0], size);

	space.write_word(0264, addr);
	space.write_word(0266, size);

	offset = 0;
	size /= 2;
	while (size-- > 0)
	{
		space.write_word(addr, little_endianize_int16(buffer[offset]));
		addr += 2;
		offset++;
	}

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

void bk_state::update_monitor_type(int state)
{
	m_monitor = state;
}

u32 bk_state::screen_update_10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 const mini = !BIT(m_scroll, 9);
	u16 const nOfs = (m_scroll & 255) + (mini ? 40 : -216);

	for (u16 y = 0; y < 256; y++)
	{
		for (u16 x = 0; x < 32; x++)
		{
			u16 const code = (y > 63 && mini) ? 0 : m_vram[((y+nOfs) %256)*32 + x];
			if (m_monitor)
			{
				for (u8 b = 0; b < 16; b += 2)
				{
					int pixel = (code >> b) & 3;
					bitmap.pix(y, x*16 + b) = pixel;
					bitmap.pix(y, x*16 + b + 1) = pixel;
				}
			}
			else
			{
				for (u8 b = 0; b < 16; b++)
					bitmap.pix(y, x*16 + b) = BIT(code, b) << 2;
			}
		}
	}
	return 0;
}
