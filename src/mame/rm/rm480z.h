// license:BSD-3-Clause
// copyright-holders:Robin Sergeant

/*

Research Machines RM 480Z

*/

#ifndef MAME_RM_RM480Z_H
#define MAME_RM_RM480Z_H

#pragma once

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"

#include "emupal.h"

//
//
//

#define RM480Z_MAINCPU_TAG      "maincpu"

//
//
//

class rm480z_state : public driver_device
{
public:
	rm480z_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_view(*this, "view"),
		m_maincpu(*this, RM480Z_MAINCPU_TAG),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ram(*this, RAM_TAG),
		m_bank(*this, "bank%u", 1U),
		m_ctc(*this, "ctc"),
		m_sio(*this, "sio"),
		m_rs232(*this, "rs232"),
		m_speaker(*this, "speaker"),
		m_io_kbrow(*this, "kbrow.%u", 0U),
		m_io_display_type(*this, "display_type"),
		m_chargen(*this, "chargen")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(monitor_changed);

	void rm480z(machine_config &config);
	void rm480za(machine_config &config);

private:

	template <int ROWS, int COLS>
	class rm480z_vram
	{
	public:
		void set_char(int row, int col, uint8_t data) { m_chars[row][col] = data; }
		void reset() { memset(m_chars, 0x80, sizeof(m_chars)); }

		uint8_t get_char(int row, int col) const { return m_chars[row][col]; }

	private:
		uint8_t m_chars[ROWS][COLS];
	};

	enum class hrg_display_mode : uint8_t
	{
		none = 0,
		high = 1,
		medium_0 = 2,
		medium_1 = 3,
		extra_high = 4
	};

	static inline constexpr int RM480Z_SCREENROWS = 24;
	static inline constexpr int RM480Z_SCREENCOLS = 80;
	static inline constexpr int RM480Z_VIDEOMODE_40COL = 0x01;
	static inline constexpr int RM480Z_VIDEOMODE_80COL = 0x02;
	static inline constexpr int RM480Z_HRG_RAM_SIZE = 16384; // 16k
	static inline constexpr int RM480Z_HRG_SCRATCHPAD_SIZE = 16;

	memory_view m_view;

	bool m_kbd_reset = true;        // cleared by firmware when it is ready to receive keyboard interrupts
	bool m_kbd_ready = false;       // set when new key code is available (on key press or release)
	uint8_t m_kbd_code = 0;         // scan code of last keyboard event (bit 7 set on key release)
	uint8_t m_kbd_scan_pos = 0;     // current keyboard matrix scanning position
	uint8_t m_kbd_state[8] = {0};   // keyboard matrix state (bits set when keys held down).

	rm480z_vram<RM480Z_SCREENROWS, RM480Z_SCREENCOLS> m_vram;
	int m_videomode = RM480Z_VIDEOMODE_80COL;
	bool m_alt_char_set = false;    // replace teletext characters with inverse ASCII characters

	uint8_t m_hrg_ram[RM480Z_HRG_RAM_SIZE];
	uint8_t m_hrg_scratchpad[RM480Z_HRG_SCRATCHPAD_SIZE];
	hrg_display_mode m_hrg_display_mode = hrg_display_mode::none;
	uint8_t m_hrg_port0 = 0;        // HRG port 0 value (various uses)
	uint8_t m_hrg_port1 = 0;        // HRG port 1 value (various uses)
	bool m_hrg_mem_open = false;    // when HRG memory is open the display in blanked (used when clearing screen)
	bool m_video_inhibit = false;   // inhibits text output on a colour display
	bool m_hrg_inhibit = false;     // inhibits graphics output on a monocrhome display

	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_memory_bank_array<4> m_bank;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<rs232_port_device> m_rs232;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<8> m_io_kbrow;
	required_ioport m_io_display_type;
	required_region_ptr<u8> m_chargen;

	TIMER_DEVICE_CALLBACK_MEMBER(kbd_scan);
	void vblank_callback(screen_device &screen, bool vblank_state);

	void rm480z_io(address_map &map);
	void rm480z_MK1_mem(address_map &map);
	void rm480z_MK2_mem(address_map &map);

	uint8_t status_port_read(offs_t offset);
	void control_port_write(offs_t offset, uint8_t data);
	uint8_t videoram_read(offs_t offset);
	void videoram_write(offs_t offset, uint8_t data);
	uint8_t hrg_port_read(offs_t offset);
	void hrg_port_write(offs_t offset, uint8_t data);

	void machine_reset() override;
	uint32_t screen_update_rm480z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_screen(bitmap_ind16 &bitmap) const;
	void putChar(int charnum, int x, int y, bitmap_ind16 &bitmap, bool bMonochrome) const;
	int calculate_hrg_vram_index() const;
	void palette_init(palette_device &palette);
	void change_palette(int index, uint8_t value);
	void draw_extra_high_res_graphics(bitmap_ind16 &bitmap) const;
	void draw_high_res_graphics(bitmap_ind16 &bitmap) const;
	void draw_medium_res_graphics(bitmap_ind16 &bitmap) const;
};

#endif // MAME_RM_RM480Z_H
