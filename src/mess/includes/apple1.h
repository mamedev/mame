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

typedef struct
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
} terminal_t;


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
};


/*----------- defined in machine/apple1.c -----------*/

extern const pia6821_interface apple1_pia0;


SNAPSHOT_LOAD( apple1 );



/*----------- defined in video/apple1.c -----------*/


SCREEN_UPDATE_IND16( apple1 );

void apple1_vh_dsp_w (running_machine &machine, int data);
void apple1_vh_dsp_clr (running_machine &machine);
attotime apple1_vh_dsp_time_to_ready (running_machine &machine);



/*----------- defined in drivers/apple1.c -----------*/

extern const gfx_layout apple1_charlayout;


#endif /* APPLE1_H_ */
