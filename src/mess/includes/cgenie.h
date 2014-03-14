/*****************************************************************************
 *
 * includes/cgenie.h
 *
 ****************************************************************************/

#ifndef CGENIE_H_
#define CGENIE_H_

#include "machine/wd17xx.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/ay8910.h"

// CRTC 6845
struct CRTC6845
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
};


class cgenie_state : public driver_device
{
public:
	cgenie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_fontram(*this, "fontram"),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_ay8910(*this, "ay8910"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{
	}

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
	UINT32 screen_update_cgenie(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cgenie_timer_interrupt);
	INTERRUPT_GEN_MEMBER(cgenie_frame_interrupt);
	TIMER_CALLBACK_MEMBER(handle_cassette_input);
	DECLARE_WRITE_LINE_MEMBER(cgenie_fdc_intrq_w);
	DECLARE_READ8_MEMBER(cgenie_sh_control_port_r);
	DECLARE_WRITE8_MEMBER(cgenie_sh_control_port_w);
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<ay8910_device> m_ay8910;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void cgenie_offset_xy();
	int cgenie_get_register(int indx);
	void cgenie_mode_select(int mode);
	void cgenie_refresh_monitor(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cgenie_refresh_tv_set(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int cgenie_port_xx_r( int offset );
	int cgenie_videoram_r( int offset );

	DECLARE_READ8_MEMBER( cgenie_psg_port_a_r );
	DECLARE_READ8_MEMBER( cgenie_psg_port_b_r );
	DECLARE_WRITE8_MEMBER( cgenie_psg_port_a_w );
	DECLARE_WRITE8_MEMBER( cgenie_psg_port_b_w );

	DECLARE_READ8_MEMBER( cgenie_colorram_r );
	DECLARE_READ8_MEMBER( cgenie_fontram_r );

	DECLARE_WRITE8_MEMBER( cgenie_colorram_w );
	DECLARE_WRITE8_MEMBER( cgenie_fontram_w );

	DECLARE_WRITE8_MEMBER( cgenie_port_ff_w );
	DECLARE_READ8_MEMBER( cgenie_port_ff_r );

	DECLARE_READ8_MEMBER( cgenie_status_r );
	DECLARE_READ8_MEMBER( cgenie_track_r );
	DECLARE_READ8_MEMBER( cgenie_sector_r );
	DECLARE_READ8_MEMBER( cgenie_data_r );

	DECLARE_WRITE8_MEMBER( cgenie_command_w );
	DECLARE_WRITE8_MEMBER( cgenie_track_w );
	DECLARE_WRITE8_MEMBER( cgenie_sector_w );
	DECLARE_WRITE8_MEMBER( cgenie_data_w );

	DECLARE_READ8_MEMBER( cgenie_irq_status_r );

	DECLARE_WRITE8_MEMBER( cgenie_motor_w );

	DECLARE_READ8_MEMBER( cgenie_keyboard_r );
	DECLARE_WRITE8_MEMBER( cgenie_videoram_w );


	DECLARE_READ8_MEMBER( cgenie_index_r );
	DECLARE_READ8_MEMBER( cgenie_register_r );

	DECLARE_WRITE8_MEMBER( cgenie_index_w );
	DECLARE_WRITE8_MEMBER( cgenie_register_w );

};


/*----------- defined in machine/cgenie.c -----------*/


extern const wd17xx_interface cgenie_wd17xx_interface;

#endif /* CGENIE_H_ */
