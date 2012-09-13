/*****************************************************************************
 *
 * includes/cgenie.h
 *
 ****************************************************************************/

#ifndef CGENIE_H_
#define CGENIE_H_

#include "machine/wd17xx.h"

// CRTC 6845
typedef struct
{
	UINT8    cursor_address_lo;
	UINT8    cursor_address_hi;
	UINT8    screen_address_lo;
	UINT8    screen_address_hi;
	UINT8    cursor_bottom;
	UINT8    cursor_top;
	UINT8    scan_lines;
	UINT8    crt_mode;
	UINT8    vertical_sync_pos;
	UINT8    vertical_displayed;
	UINT8    vertical_adjust;
	UINT8    vertical_total;
	UINT8    horizontal_length;
	UINT8    horizontal_sync_pos;
	UINT8    horizontal_displayed;
	UINT8    horizontal_total;
	UINT8    idx;
	UINT8    cursor_visible;
	UINT8    cursor_phase;
} CRTC6845;


class cgenie_state : public driver_device
{
public:
	cgenie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_colorram(*this, "colorram"),
		m_fontram(*this, "fontram"){ }

	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_fontram;

	UINT8 *m_videoram;
	int m_tv_mode;
	int m_font_offset[4];
	int m_port_ff;
	UINT8 m_irq_status;
	UINT8 m_motor_drive;
	UINT8 m_head;
	UINT8 m_cass_level;
	UINT8 m_cass_bit;
	UINT8 m_psg_a_out;
	UINT8 m_psg_b_out;
	UINT8 m_psg_a_inp;
	UINT8 m_psg_b_inp;
	UINT8 m_control_port;
	CRTC6845 m_crt;
	int m_graphics;
	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_dlybitmap;
	int m_off_x;
	int m_off_y;
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(cgenie);
	DECLARE_PALETTE_INIT(cgenienz);
};


/*----------- defined in machine/cgenie.c -----------*/

READ8_DEVICE_HANDLER( cgenie_sh_control_port_r );
WRITE8_DEVICE_HANDLER( cgenie_sh_control_port_w );

extern const wd17xx_interface cgenie_wd17xx_interface;


READ8_HANDLER ( cgenie_psg_port_a_r );
READ8_HANDLER ( cgenie_psg_port_b_r );
WRITE8_HANDLER ( cgenie_psg_port_a_w );
WRITE8_HANDLER ( cgenie_psg_port_b_w );




READ8_HANDLER ( cgenie_colorram_r );
READ8_HANDLER ( cgenie_fontram_r );

WRITE8_HANDLER ( cgenie_colorram_w );
WRITE8_HANDLER ( cgenie_fontram_w );

WRITE8_HANDLER ( cgenie_port_ff_w );
 READ8_HANDLER ( cgenie_port_ff_r );
int cgenie_port_xx_r(int offset);

INTERRUPT_GEN( cgenie_timer_interrupt );
INTERRUPT_GEN( cgenie_frame_interrupt );

 READ8_HANDLER ( cgenie_status_r );
 READ8_HANDLER ( cgenie_track_r );
 READ8_HANDLER ( cgenie_sector_r );
 READ8_HANDLER ( cgenie_data_r );

WRITE8_HANDLER ( cgenie_command_w );
WRITE8_HANDLER ( cgenie_track_w );
WRITE8_HANDLER ( cgenie_sector_w );
WRITE8_HANDLER ( cgenie_data_w );

 READ8_HANDLER ( cgenie_irq_status_r );

WRITE8_HANDLER ( cgenie_motor_w );

 READ8_HANDLER ( cgenie_keyboard_r );
int cgenie_videoram_r(running_machine &machine,int offset);
WRITE8_HANDLER ( cgenie_videoram_w );


/*----------- defined in video/cgenie.c -----------*/


SCREEN_UPDATE_IND16( cgenie );

READ8_HANDLER ( cgenie_index_r );
READ8_HANDLER ( cgenie_register_r );

WRITE8_HANDLER ( cgenie_index_w );
WRITE8_HANDLER ( cgenie_register_w );

int cgenie_get_register(running_machine &machine, int indx);
void cgenie_mode_select(running_machine &machine, int graphics);


#endif /* CGENIE_H_ */
