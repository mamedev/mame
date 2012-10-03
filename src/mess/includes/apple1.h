/*****************************************************************************
 *
 * includes/apple1.h
 *
 ****************************************************************************/

#ifndef APPLE1_H_
#define APPLE1_H_

#include "imagedev/snapquik.h"
#include "machine/6821pia.h"

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
		: driver_device(mconfig, type, tag) { }

	int m_vh_clrscrn_pressed;
	int m_kbd_data;
	UINT32 m_kbd_last_scan[4];
	int m_reset_flag;
	int m_cassette_output_flipflop;
	terminal_t *m_current_terminal;
	terminal_t *m_terminal;
	int m_blink_on;
	DECLARE_READ8_MEMBER(apple1_cassette_r);
	DECLARE_WRITE8_MEMBER(apple1_cassette_w);
	DECLARE_DRIVER_INIT(apple1);
	TILE_GET_INFO_MEMBER(terminal_gettileinfo);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_apple1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(apple1_kbd_poll);
	TIMER_CALLBACK_MEMBER(apple1_kbd_strobe_end);
	TIMER_CALLBACK_MEMBER(apple1_dsp_ready_start);
	TIMER_CALLBACK_MEMBER(apple1_dsp_ready_end);
	DECLARE_READ8_MEMBER(apple1_pia0_kbdin);
	DECLARE_WRITE8_MEMBER(apple1_pia0_dspout);
	DECLARE_WRITE8_MEMBER(apple1_pia0_dsp_write_signal);
};


/*----------- defined in machine/apple1.c -----------*/

extern const pia6821_interface apple1_pia0;
SNAPSHOT_LOAD( apple1 );

/*----------- defined in video/apple1.c -----------*/
void apple1_vh_dsp_w (running_machine &machine, int data);
void apple1_vh_dsp_clr (running_machine &machine);
attotime apple1_vh_dsp_time_to_ready (running_machine &machine);


/*----------- defined in drivers/apple1.c -----------*/

extern const gfx_layout apple1_charlayout;


#endif /* APPLE1_H_ */
