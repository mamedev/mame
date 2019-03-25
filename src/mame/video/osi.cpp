// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert, Wilbert Pol
#include "emu.h"
#include "includes/osi.h"
#include "screen.h"

/* Palette Initialization */

void sb2m600_state::osi630_palette(palette_device &palette) const
{
	/* black and white */
	palette.set_pen_color(0, 0x00, 0x00, 0x00); // black
	palette.set_pen_color(1, 0xff, 0xff, 0xff); // white

	/* color enabled */
	palette.set_pen_color(2, 0xff, 0xff, 0x00); // yellow
	palette.set_pen_color(3, 0xff, 0x00, 0x00); // red
	palette.set_pen_color(4, 0x00, 0xff, 0x00); // green
	palette.set_pen_color(5, 0x00, 0x80, 0x00); // olive green
	palette.set_pen_color(6, 0x00, 0x00, 0xff); // blue
	palette.set_pen_color(7, 0xff, 0x00, 0xff); // purple
	palette.set_pen_color(8, 0x00, 0x00, 0x80); // sky blue
	palette.set_pen_color(9, 0x00, 0x00, 0x00); // black
}

/* Video Start */

void sb2m600_state::video_start()
{
	// randomize video memory contents
	for (uint16_t addr = 0; addr < OSI600_VIDEORAM_SIZE; addr++)
		m_video_ram[addr] = machine().rand() & 0xff;

	// randomize color memory contents
	if (m_color_ram)
		for (uint16_t addr = 0; addr < OSI630_COLORRAM_SIZE; addr++)
			m_color_ram[addr] = machine().rand() & 0x0f;
}

/* Video Update */

uint32_t sb2m600_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_32)
	{
		for (int y = 0; y < 256; y++)
		{
			uint16_t videoram_addr = (y >> 4) * 64;
			int const line = (y >> 1) & 0x07;
			int x = 0;

			for (int sx = 0; sx < 64; sx++)
			{
				uint8_t videoram_data = m_video_ram[videoram_addr];
				uint16_t charrom_addr = ((videoram_data << 3) | line) & 0x7ff;
				uint8_t charrom_data = m_p_chargen[charrom_addr];

				for (int bit = 0; bit < 8; bit++)
				{
					bool color = BIT(charrom_data, 7);

					if (m_coloren)
					{
						uint8_t colorram_data = m_color_ram[videoram_addr];
						color = (color ^ BIT(colorram_data, 0)) ? (((colorram_data >> 1) & 0x07) + 2) : 0;
					}

					bitmap.pix16(y, x++) = color;

					charrom_data <<= 1;
				}

				videoram_addr++;
			}
		}
	}
	else
	{
		for (int y = 0; y < 256; y++)
		{
			uint16_t videoram_addr = (y >> 3) * 32;
			int const line = y & 0x07;
			int x = 0;

			for (int sx = 0; sx < 32; sx++)
			{
				uint8_t videoram_data = m_video_ram[videoram_addr];
				uint16_t const charrom_addr = ((videoram_data << 3) | line) & 0x7ff;
				uint8_t charrom_data = m_p_chargen[charrom_addr];

				for (int bit = 0; bit < 8; bit++)
				{
					bool color = BIT(charrom_data, 7);

					if (m_coloren)
					{
						uint8_t colorram_data = m_color_ram[videoram_addr];
						color = (color ^ BIT(colorram_data, 0)) ? (((colorram_data >> 1) & 0x07) + 2) : 0;
					}

					bitmap.pix16(y, x++) = color;
					bitmap.pix16(y, x++) = color;

					charrom_data <<= 1;
				}

				videoram_addr++;
			}
		}
	}

	return 0;
}

uint32_t uk101_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 256; y++)
	{
		uint16_t videoram_addr = (y >> 4) * 64;
		int const line = (y >> 1) & 0x07;
		int x = 0;

		for (int sx = 0; sx < 64; sx++)
		{
			uint8_t videoram_data = m_video_ram[videoram_addr++];
			uint16_t const charrom_addr = ((videoram_data << 3) | line) & 0x7ff;
			uint8_t charrom_data = m_p_chargen[charrom_addr];

			for (int bit = 0; bit < 8; bit++)
			{
				bitmap.pix16(y, x) = BIT(charrom_data, 7);
				x++;
				charrom_data <<= 1;
			}
		}
	}

	return 0;
}

/* Machine Drivers */

MACHINE_CONFIG_START(sb2m600_state::osi600_video)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(X1/256/256) // 60 Hz
	MCFG_SCREEN_UPDATE_DRIVER(sb2m600_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", palette_device::MONOCHROME);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(uk101_state::uk101_video)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DRIVER(uk101_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 16*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 16*16-1)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", palette_device::MONOCHROME);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sb2m600_state::osi630_video)
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(X1/256/256) // 60 Hz
	MCFG_SCREEN_UPDATE_DRIVER(sb2m600_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 16*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 16*16-1)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(sb2m600_state::osi630_palette), 8+2);
MACHINE_CONFIG_END
