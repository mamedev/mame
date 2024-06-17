// license:BSD-3-Clause
// copyright-holders:Robin Sergeant

/*

Research Machines RM 480Z

*/

#ifndef MAME_RM_RM480Z_H
#define MAME_RM_RM480Z_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
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
		m_messram(*this, RAM_TAG),
		m_ctc(*this, "ctc"),
		m_speaker(*this, "speaker"),
		m_io_kbrow(*this, "kbrow.%u", 0U),
		m_chargen(*this, "chargen")
	{
	}

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

	static inline constexpr int RM480Z_SCREENROWS = 24;
	static inline constexpr int RM480Z_SCREENCOLS = 80;
	static inline constexpr int RM480Z_VIDEOMODE_40COL = 0x01;
	static inline constexpr int RM480Z_VIDEOMODE_80COL = 0x02;

	memory_view m_view;

	bool m_kbd_reset = true;
	bool m_kbd_ready = false;
	uint8_t m_kbd_code = 0;
	uint8_t m_kbd_scan_pos = 0;
	uint8_t m_kbd_state[8] = {0};

	rm480z_vram<RM480Z_SCREENROWS, RM480Z_SCREENCOLS> m_vram;
	int m_videomode = RM480Z_VIDEOMODE_80COL;
	bool m_alt_char_set = false;

	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ram_device> m_messram;
	required_device<z80ctc_device> m_ctc;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<8> m_io_kbrow;
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
	void putChar(int charnum, int x, int y, bitmap_ind16 &bitmap) const;
	void config_videomode(bool b80col);
};

#endif // MAME_RM_RM480Z_H
