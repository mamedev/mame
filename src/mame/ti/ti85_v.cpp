// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha,Jon Sturm,grubbyplaya
/***************************************************************************
  TI-85 driver by Krzysztof Strzecha

  Functions to emulate the video hardware of the TI-85

***************************************************************************/

#include "emu.h"
#include "ti85.h"

#define TI_VIDEO_COL_SIZE       160 / 8
#define TI_VIDEO_MEMORY_SIZE    TI_VIDEO_COL_SIZE * 64

#define TI81_SCREEN_X_SIZE      12
#define TI81_SCREEN_Y_SIZE      64

#define TI85_SCREEN_X_SIZE      16
#define TI85_SCREEN_Y_SIZE      64

#define TI86_SCREEN_X_SIZE      16
#define TI86_SCREEN_Y_SIZE      64

static constexpr rgb_t ti85_colors[64] =
{
    rgb_t(174, 205, 176), rgb_t(152, 181, 167), //0x00
    rgb_t(174, 205, 176), rgb_t(146, 175, 164), //0x01
    rgb_t(174, 205, 176), rgb_t(140, 169, 162), //0x02
    rgb_t(174, 205, 176), rgb_t(134, 163, 159), //0x03
    rgb_t(174, 205, 176), rgb_t(129, 157, 157), //0x04
    rgb_t(174, 205, 176), rgb_t(123, 151, 154), //0x05
    rgb_t(174, 205, 176), rgb_t(117, 146, 152), //0x06
    rgb_t(174, 205, 176), rgb_t(111, 140, 150), //0x07
    rgb_t(174, 205, 176), rgb_t(106, 134, 147), //0x08
    rgb_t(174, 205, 176), rgb_t(100, 128, 145), //0x09
    rgb_t(174, 205, 176), rgb_t(94, 122, 142),  //0x0a
    rgb_t(174, 205, 176), rgb_t(88, 116, 140),  //0x0b
    rgb_t(174, 205, 176), rgb_t(83, 111, 138),  //0x0c
    rgb_t(169, 200, 174), rgb_t(83, 111, 138),  //0x0d
    rgb_t(164, 195, 172), rgb_t(83, 111, 138),  //0x0e
    rgb_t(160, 190, 170), rgb_t(83, 111, 138),  //0x0f
    rgb_t(155, 186, 168), rgb_t(83, 111, 138),  //0x10
    rgb_t(151, 181, 166), rgb_t(83, 111, 138),  //0x11
    rgb_t(146, 176, 164), rgb_t(83, 111, 138),  //0x12
    rgb_t(142, 172, 162), rgb_t(83, 111, 138),  //0x13
    rgb_t(137, 167, 160), rgb_t(83, 111, 138),  //0x14
    rgb_t(133, 162, 158), rgb_t(83, 111, 138),  //0x15
    rgb_t(128, 158, 157), rgb_t(83, 111, 138),  //0x16
    rgb_t(124, 153, 155), rgb_t(83, 111, 138),  //0x17
    rgb_t(119, 148, 153), rgb_t(83, 111, 138),  //0x18
    rgb_t(115, 144, 151), rgb_t(83, 111, 138),  //0x19
    rgb_t(110, 139, 149), rgb_t(83, 111, 138),  //0x1a
    rgb_t(106, 134, 147), rgb_t(83, 111, 138),  //0x1b
    rgb_t(101, 130, 145), rgb_t(83, 111, 138),  //0x1c
    rgb_t(97, 125, 143),  rgb_t(83, 111, 138),  //0x1d
    rgb_t(92, 120, 141),  rgb_t(83, 111, 138),  //0x1e
    rgb_t(87, 116, 140),  rgb_t(83, 111, 138)   //0x1f
};

static constexpr rgb_t ti82_colors[128] =
{
    rgb_t(84, 111, 138),   rgb_t(83, 111, 138),  //0x00
    rgb_t(86, 114, 138),   rgb_t(83, 111, 138),  //0x01
    rgb_t(89, 116, 139),   rgb_t(83, 111, 138),  //0x02
    rgb_t(91, 119, 141),   rgb_t(83, 111, 138),  //0x03
    rgb_t(94, 122, 142),   rgb_t(83, 111, 138),  //0x04
    rgb_t(95, 124, 142),   rgb_t(83, 111, 138),  //0x05
    rgb_t(97, 127, 143),   rgb_t(83, 111, 138),  //0x06
    rgb_t(100, 128, 145),  rgb_t(83, 111, 138),  //0x07
    rgb_t(102, 131, 146),  rgb_t(83, 111, 138),  //0x08
    rgb_t(105, 133, 146),  rgb_t(83, 111, 138),  //0x09
    rgb_t(107, 136, 148),  rgb_t(83, 111, 138),  //0x0a
    rgb_t(110, 138, 149),  rgb_t(83, 111, 138),  //0x0b
    rgb_t(112, 141, 149),  rgb_t(83, 111, 138),  //0x0c
    rgb_t(114, 144, 150),  rgb_t(83, 111, 138),  //0x0d
    rgb_t(117, 146, 152),  rgb_t(83, 111, 138),  //0x0e
    rgb_t(119, 149, 153),  rgb_t(83, 111, 138),  //0x0f
    rgb_t(122, 150, 153),  rgb_t(83, 111, 138),  //0x10
    rgb_t(124, 153, 154),  rgb_t(83, 111, 138),  //0x11
    rgb_t(127, 155, 156),  rgb_t(83, 111, 138),  //0x12
    rgb_t(129, 158, 157),  rgb_t(83, 111, 138),  //0x13
    rgb_t(131, 161, 157),  rgb_t(83, 111, 138),  //0x14
    rgb_t(134, 163, 158),  rgb_t(83, 111, 138),  //0x15
    rgb_t(136, 166, 160),  rgb_t(83, 111, 138),  //0x16
    rgb_t(139, 168, 160),  rgb_t(83, 111, 138),  //0x17
    rgb_t(141, 170, 161),  rgb_t(83, 111, 138),  //0x18
    rgb_t(144, 172, 162),  rgb_t(83, 111, 138),  //0x19
    rgb_t(145, 175, 164),  rgb_t(83, 111, 138),  //0x1a
    rgb_t(147, 178, 164),  rgb_t(83, 111, 138),  //0x1b
    rgb_t(150, 180, 165),  rgb_t(83, 111, 138),  //0x1c
    rgb_t(152, 183, 166),  rgb_t(83, 111, 138),  //0x1d
    rgb_t(155, 185, 168),  rgb_t(83, 111, 138),  //0x1e
    rgb_t(158, 187, 168),  rgb_t(83, 111, 138),  //0x1f
    rgb_t(159, 188, 169),  rgb_t(83, 111, 138),  //0x20
    rgb_t(160, 190, 170),  rgb_t(83, 111, 138),  //0x21
    rgb_t(160, 190, 170),  rgb_t(83, 111, 138),  //0x22
    rgb_t(160, 190, 170),  rgb_t(83, 111, 138),  //0x23
    rgb_t(160, 190, 170),  rgb_t(83, 112, 138),  //0x24
    rgb_t(160, 190, 170),  rgb_t(84, 112, 138),  //0x25
    rgb_t(160, 190, 170),  rgb_t(85, 113, 139),  //0x26
    rgb_t(160, 190, 170),  rgb_t(87, 114, 139),  //0x27
    rgb_t(160, 190, 170),  rgb_t(88, 116, 139),  //0x28
    rgb_t(160, 190, 170),  rgb_t(89, 118, 140),  //0x29
    rgb_t(160, 190, 170),  rgb_t(92, 119, 141),  //0x2a
    rgb_t(160, 190, 170),  rgb_t(93, 121, 141),  //0x2b
    rgb_t(160, 190, 170),  rgb_t(95, 124, 142),  //0x2c
    rgb_t(160, 190, 170),  rgb_t(97, 127, 143),  //0x2d
    rgb_t(160, 190, 170),  rgb_t(101, 129, 144), //0x2e
    rgb_t(160, 190, 170),  rgb_t(104, 132, 145), //0x2f
    rgb_t(160, 190, 170),  rgb_t(106, 135, 147), //0x30
    rgb_t(160, 190, 170),  rgb_t(109, 138, 148), //0x31
    rgb_t(160, 190, 170),  rgb_t(112, 141, 149), //0x32
    rgb_t(160, 190, 170),  rgb_t(116, 145, 151), //0x33
    rgb_t(160, 190, 170),  rgb_t(120, 150, 153), //0x34
    rgb_t(160, 190, 170),  rgb_t(124, 153, 154), //0x35
    rgb_t(160, 190, 170),  rgb_t(129, 158, 157), //0x36
    rgb_t(160, 190, 170),  rgb_t(133, 161, 158), //0x37
    rgb_t(160, 190, 170),  rgb_t(137, 167, 160), //0x38
    rgb_t(160, 190, 170),  rgb_t(142, 171, 162), //0x39
    rgb_t(160, 190, 170),  rgb_t(147, 177, 164), //0x3a
    rgb_t(160, 190, 170),  rgb_t(152, 181, 166), //0x3b
    rgb_t(160, 190, 170),  rgb_t(157, 188, 168), //0x3c
    rgb_t(160, 190, 170),  rgb_t(160, 190, 170), //0x3d
    rgb_t(160, 190, 170),  rgb_t(160, 190, 170), //0x3e
    rgb_t(160, 190, 170),  rgb_t(160, 190, 170), //0x3f
};

TIMER_CALLBACK_MEMBER(ti85_state::lcd_dma_callback)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
    static uint8_t last_rowh = 0;
    static const uint8_t dma_width[4] = {10, 12, 16, 20};
    const uint8_t row_bytes = dma_width[m_video_buffer_width];
    const uint8_t row_count = (m_video_buffer_rows + 1) * 16;
	const int lcdmem = (((m_LCD_memory_base & 0x3F) + 0xc0) << 8) + ((m_lcdrow % row_count) * row_bytes);

    uint8_t *row = m_frame + m_lcdrow * TI_VIDEO_COL_SIZE;
    for (int x = 0; x < 10; x++)
        row[x] = space.read_byte(lcdmem + x);

    for (int x = 10; x < row_bytes; x++)
        row[TI_VIDEO_COL_SIZE - row_bytes + x] = space.read_byte(lcdmem + x);

    if (m_video_buffer_width != 3)
        row[10] = (row[10] & 0x0f) | last_rowh;

    last_rowh = row[0] & 0xf0;

    // wait dma_speed*row_bytes M cycles
    m_maincpu->adjust_icount((m_LCD_dma_speed ? 6 : 2) * row_bytes * -4);
    
    // trigger an interrupt one line after a frame is 1/4 and 3/4 of the way done
    if ((m_lcdrow - 1 == row_count / 4) || (m_lcdrow - 1 == row_count * 3 / 4))
    {
        if (m_LCD_mask)
		    m_maincpu->set_input_line(0, HOLD_LINE);
        m_LCD_interrupt_status = 1;
    }

    m_lcdrow = (m_lcdrow + 1) % m_lcdheight;
}

void ti85_state::ti85_palette(palette_device &palette)
{
	for (int i = 0; i < 32 * 2; i++)
		palette.set_pen_color(i, ti85_colors[i]);

	if (!strncmp(machine().system().name, "ti81", 4))
	{
		m_lcdwidth = TI81_SCREEN_X_SIZE;
		m_lcdheight = TI81_SCREEN_Y_SIZE;
	}
	else if (!strncmp(machine().system().name, "ti85", 4))
	{
		m_lcdwidth = TI85_SCREEN_X_SIZE;
		m_lcdheight = TI85_SCREEN_Y_SIZE;
	}
	else if (!strncmp(machine().system().name, "ti86", 4))
	{
		m_lcdwidth = TI86_SCREEN_X_SIZE;
		m_lcdheight = TI86_SCREEN_Y_SIZE;
	}
	else
	{
		// don't allocate memory for the others drivers
		return;
	}
}

uint32_t ti85_state::screen_update_ti85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_LCD_status)
	{
        bitmap.fill((m_LCD_contrast & 0x1f) * 2, cliprect);
		return 0;
	}

    const uint8_t contrast = (m_LCD_contrast & 0x1f) * 2;
    for (int y = 0; y < m_lcdheight; y++)
    {
        for (int x = 0; x < m_lcdwidth; x++)
        {
            int src = x < 10 ? x : TI_VIDEO_COL_SIZE - m_lcdwidth + x;
            for (int b = 0; b < 8; b++)
                bitmap.pix(y, x * 8 + b) = contrast + BIT(m_frame[y * TI_VIDEO_COL_SIZE + src], 7 - b);
        }
    }

	return 0;
}

void ti85_state::ti82_palette(palette_device &palette) const
{
    for (int i = 0; i < 64 * 2; i++)
        palette.set_pen_color(i, ti82_colors[i]);
}
