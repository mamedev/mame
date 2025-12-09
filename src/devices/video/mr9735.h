// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    MR9735 Teletext/Viewdata 625-line Video Generator

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  Vdd
    /Chip Select Input   2 |             | 39  Vcc
                    A0   3 |             | 38  Composite Sync Input
                    A1   4 |             | 37  Composite Sync Output
                    A2   5 |             | 36  Line Flyback Input
                    A3   6 |             | 35  Interlace Select Input
                    A4   7 |             | 34  /D7
                    A5   8 |             | 33  /D6
                    A6   9 |             | 32  /D5
                    A7  10 |    MR9735   | 31  /D4
                    A8  11 |             | 30  /D3
                    A9  12 |             | 29  /D2
     Read/Write Output  13 |             | 28  /D1
            SS0 Output  14 |             | 27  /D0
            SS1 Output  15 |             | 26  Picture Text Output
            SS2 Output  16 |             | 25  Red Gun Output
            TS1 Output  17 |             | 24  Green Gun Output
            TS2 Output  18 |             | 23  Blue Gun Output
        19.2kHz Output  19 |             | 22  /RSYNC Output
         1.2kHz Output  20 |_____________| 21  6MHz Input

**********************************************************************/

#ifndef MAME_VIDEO_MR9735_H
#define MAME_VIDEO_MR9735_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mr9735_002_device

class mr9735_002_device : public device_t
{
public:
	// construction/destruction
	mr9735_002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_interlace(bool interlace) { m_interlace = interlace; }

	auto data_cb() { return m_read_data.bind(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	mr9735_002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	enum
	{
		NUL = 0,
		ALPHA_RED,
		ALPHA_GREEN,
		ALPHA_YELLOW,
		ALPHA_BLUE,
		ALPHA_MAGENTA,
		ALPHA_CYAN,
		ALPHA_WHITE,
		FLASH,
		STEADY,
		END_BOX,
		START_BOX,
		NORMAL_HEIGHT,
		DOUBLE_HEIGHT,
		SPECIAL_GFX,
		NORMAL_GFX,
		DLE,
		GRAPHICS_RED,
		GRAPHICS_GREEN,
		GRAPHICS_YELLOW,
		GRAPHICS_BLUE,
		GRAPHICS_MAGENTA,
		GRAPHICS_CYAN,
		GRAPHICS_WHITE,
		CONCEAL_DISPLAY,
		CONTIGUOUS_GFX,
		SEPARATED_GFX,
		ESC,
		BLACK_BACKGROUND,
		NEW_BACKGROUND,
		HOLD_GRAPHICS,
		RELEASE_GRAPHICS
	};

	static constexpr uint8_t ALPHANUMERIC = 0x01;
	static constexpr uint8_t CONTIGUOUS   = 0x02;
	static constexpr uint8_t SEPARATED    = 0x03;

	void lfb_w(int state);
	void process_control_character(uint8_t data);
	void set_next_chartype();
	uint16_t get_gfx_data(uint8_t data, offs_t row, bool separated);
	uint16_t get_rom_data(uint8_t data, offs_t row);
	uint16_t character_rounding(uint16_t a, uint16_t b);
	uint16_t get_character_data(uint8_t data);

	required_region_ptr<uint8_t> m_char_rom;

	devcb_read8 m_read_data;

	uint8_t m_held_char;
	uint8_t m_next_chartype;
	uint8_t m_curr_chartype;
	uint8_t m_held_chartype;
	uint16_t m_ra;
	uint8_t m_bg;
	uint8_t m_fg;
	uint8_t m_prev_col;
	bool m_graphics;
	bool m_separated;
	bool m_flash;
	bool m_boxed;
	bool m_dbl_height;
	bool m_dbl_height_bottom_row;
	bool m_dbl_height_prev_row;
	bool m_hold_char;
	uint8_t m_frame_count;

	static constexpr int NUM_COLS = 40;
	static constexpr int NUM_ROWS = 24;

	bool m_interlace;
	uint8_t m_char_rows;
};


// device type definition
DECLARE_DEVICE_TYPE(MR9735_002, mr9735_002_device) // MR9735-002 English


#endif // MAME_VIDEO_MR9735_H
