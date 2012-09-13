/*****************************************************************************
 *
 * includes/ti85.h
 *
 ****************************************************************************/

#ifndef TI85_H_
#define TI85_H_

#include "imagedev/snapquik.h"
#include "video/t6a04.h"
#include "sound/speaker.h"


class ti85_state : public driver_device
{
public:
	ti85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_speaker(*this, SPEAKER_TAG),
//        m_serial(*this, "tiserial"),
		  m_nvram(*this, "nvram")
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<device_t> m_speaker;
	//optional_device<device_t> m_serial;
	optional_shared_ptr<UINT8>	m_nvram;

	UINT8 m_LCD_memory_base;
	UINT8 m_LCD_contrast;
	UINT8 m_LCD_status;
	UINT8 m_timer_interrupt_mask;
	UINT8 m_timer_interrupt_status;
	UINT8 m_ON_interrupt_mask;
	UINT8 m_ON_interrupt_status;
	UINT8 m_ON_pressed;
	UINT8 m_ti8x_memory_page_1;
	UINT8 m_ti8x_memory_page_2;
	UINT8 m_LCD_mask;
	UINT8 m_power_mode;
	UINT8 m_keypad_mask;
	UINT8 m_video_buffer_width;
	UINT8 m_interrupt_speed;
	UINT8 m_port4_bit0;
	UINT8 m_ti81_port_7_data;
	UINT8 *m_ti8x_ram;
	UINT8 m_PCR;
	UINT8 m_red_out;
	UINT8 m_white_out;
	UINT8 m_ti8x_port2;
	UINT8 m_ti83p_port4;
	int m_ti_video_memory_size;
	int m_ti_screen_x_size;
	int m_ti_screen_y_size;
	int m_ti_number_of_frames;
	UINT8 * m_frames;
	UINT8 * m_bios;
	DECLARE_READ8_MEMBER(ti85_port_0000_r);
	DECLARE_READ8_MEMBER(ti8x_keypad_r);
	DECLARE_READ8_MEMBER(ti85_port_0006_r);
	DECLARE_READ8_MEMBER(ti8x_serial_r);
	DECLARE_READ8_MEMBER(ti86_port_0005_r);
	DECLARE_READ8_MEMBER(ti83_port_0000_r);
	DECLARE_READ8_MEMBER(ti8x_plus_serial_r);
	DECLARE_WRITE8_MEMBER(ti81_port_0007_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0000_w);
	DECLARE_WRITE8_MEMBER(ti8x_keypad_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0003_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0004_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0005_w);
	DECLARE_WRITE8_MEMBER(ti85_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti8x_serial_w);
	DECLARE_WRITE8_MEMBER(ti86_port_0005_w);
	DECLARE_WRITE8_MEMBER(ti86_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti82_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti83_port_0000_w);
	DECLARE_WRITE8_MEMBER(ti83_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti83_port_0003_w);
	DECLARE_WRITE8_MEMBER(ti8x_plus_serial_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0002_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0003_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0004_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0006_w);
	DECLARE_WRITE8_MEMBER(ti83p_port_0007_w);
	DECLARE_READ8_MEMBER( ti85_port_0002_r );
	DECLARE_READ8_MEMBER( ti85_port_0003_r );
	DECLARE_READ8_MEMBER( ti85_port_0004_r );
	DECLARE_READ8_MEMBER( ti85_port_0005_r );
	DECLARE_READ8_MEMBER( ti86_port_0006_r );
	DECLARE_READ8_MEMBER( ti82_port_0002_r );
	DECLARE_READ8_MEMBER( ti83_port_0002_r );
	DECLARE_READ8_MEMBER( ti83_port_0003_r );
	DECLARE_READ8_MEMBER( ti83p_port_0002_r );
	virtual void machine_start();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_RESET(ti85);
	DECLARE_PALETTE_INIT(ti82);
	DECLARE_MACHINE_START(ti86);
	DECLARE_MACHINE_START(ti83p);
};


/*----------- defined in machine/ti85.c -----------*/






NVRAM_HANDLER( ti83p );
NVRAM_HANDLER( ti86 );

SNAPSHOT_LOAD( ti8x );


/*----------- defined in video/ti85.c -----------*/


SCREEN_UPDATE_IND16( ti85 );




#endif /* TI85_H_ */
