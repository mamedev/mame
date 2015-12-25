// license:???
// copyright-holders:Paul Daniels, Colin Howell, R. Belmont
/*****************************************************************************
 *
 * includes/apple1.h
 *
 ****************************************************************************/

#ifndef APPLE1_H_
#define APPLE1_H_

#include "imagedev/snapquik.h"
#include "machine/ram.h"

typedef short termchar_t;

struct terminal_t
{
	tilemap_t *tm;
	int gfx;
	int blank_char;
	int char_bits;
	int num_cols;
	int num_rows;
	int (*getcursorcode)(int original_code);
	int cur_offset;
	int cur_hidden;
	termchar_t mem[1];
};


class apple1_state : public driver_device
{
public:
	apple1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	int m_vh_clrscrn_pressed;
	int m_kbd_data;
	UINT32 m_kbd_last_scan[4];
	int m_reset_flag;
	terminal_t *m_current_terminal;
	terminal_t *m_terminal;
	int m_blink_on;
	DECLARE_DRIVER_INIT(apple1);
	TILE_GET_INFO_MEMBER(terminal_gettileinfo);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_apple1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(apple1_kbd_poll);
	TIMER_CALLBACK_MEMBER(apple1_kbd_strobe_end);
	TIMER_CALLBACK_MEMBER(apple1_dsp_ready_start);
	TIMER_CALLBACK_MEMBER(apple1_dsp_ready_end);
	DECLARE_READ8_MEMBER(apple1_pia0_kbdin);
	DECLARE_WRITE8_MEMBER(apple1_pia0_dspout);
	DECLARE_WRITE_LINE_MEMBER(apple1_pia0_dsp_write_signal);
	required_device<cpu_device> m_maincpu;
	void terminal_draw(screen_device &screen, bitmap_ind16 &dest, const rectangle &cliprect, terminal_t *terminal);
	void verify_coords(terminal_t *terminal, int x, int y);
	void terminal_putchar(terminal_t *terminal, int x, int y, int ch);
	int terminal_getchar(terminal_t *terminal, int x, int y);
	void terminal_putblank(terminal_t *terminal, int x, int y);
	void terminal_dirtycursor(terminal_t *terminal);
	void terminal_setcursor(terminal_t *terminal, int x, int y);
	void terminal_hidecursor(terminal_t *terminal);
	void terminal_showcursor(terminal_t *terminal);
	void terminal_getcursor(terminal_t *terminal, int *x, int *y);
	void terminal_fill(terminal_t *terminal, int val);
	void terminal_clear(terminal_t *terminal);
	void apple1_vh_dsp_w (int data);
	void apple1_vh_dsp_clr ();
	void apple1_vh_cursor_blink ();
	int apple1_verify_header (UINT8 *data);
	terminal_t *terminal_create(int gfx, int blank_char, int char_bits,int (*getcursorcode)(int original_code),int num_cols, int num_rows);
	attotime apple1_vh_dsp_time_to_ready();
	DECLARE_SNAPSHOT_LOAD_MEMBER( apple1 );
	required_device<ram_device> m_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
};


/*----------- defined in drivers/apple1.c -----------*/

extern const gfx_layout apple1_charlayout;


#endif /* APPLE1_H_ */
