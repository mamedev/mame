/******************************************************************************
 *  Sharp MZ700
 *
 *  variables and function prototypes
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Reference: http://sharpmz.computingmuseum.com
 *
 ******************************************************************************/

#ifndef MZ700_H_
#define MZ700_H_

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/z80pio.h"


class mz_state : public driver_device
{
public:
	mz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_mz700;                /* 1 if running on an mz700 */

	device_t *m_pit;
	i8255_device *m_ppi;

	int m_cursor_timer;
	int m_other_timer;

	int m_intmsk;   /* PPI8255 pin PC2 */

	int m_mz700_ram_lock;       /* 1 if ram lock is active */
	int m_mz700_ram_vram;       /* 1 if vram is banked in */

	/* mz800 specific */
	UINT8 *m_cgram;

	int m_mz700_mode;           /* 1 if in mz700 mode */
	int m_mz800_ram_lock;       /* 1 if lock is active */
	int m_mz800_ram_monitor;    /* 1 if monitor rom banked in */

	int m_hires_mode;           /* 1 if in 640x200 mode */
	int m_screen;           /* screen designation */
	UINT8 *m_colorram;
	UINT8 *m_videoram;
	UINT8 m_speaker_level;
	UINT8 m_prev_state;
	UINT16 m_mz800_ramaddr;
	UINT8 m_mz800_palette[4];
	UINT8 m_mz800_palette_bank;
	DECLARE_READ8_MEMBER(mz700_e008_r);
	DECLARE_WRITE8_MEMBER(mz700_e008_w);
	DECLARE_READ8_MEMBER(mz800_bank_0_r);
	DECLARE_WRITE8_MEMBER(mz700_bank_0_w);
	DECLARE_WRITE8_MEMBER(mz800_bank_0_w);
	DECLARE_READ8_MEMBER(mz800_bank_1_r);
	DECLARE_WRITE8_MEMBER(mz700_bank_1_w);
	DECLARE_WRITE8_MEMBER(mz700_bank_2_w);
	DECLARE_WRITE8_MEMBER(mz700_bank_3_w);
	DECLARE_WRITE8_MEMBER(mz700_bank_4_w);
	DECLARE_WRITE8_MEMBER(mz700_bank_5_w);
	DECLARE_WRITE8_MEMBER(mz700_bank_6_w);
	DECLARE_READ8_MEMBER(mz800_crtc_r);
	DECLARE_WRITE8_MEMBER(mz800_write_format_w);
	DECLARE_WRITE8_MEMBER(mz800_read_format_w);
	DECLARE_WRITE8_MEMBER(mz800_display_mode_w);
	DECLARE_WRITE8_MEMBER(mz800_scroll_border_w);
	DECLARE_READ8_MEMBER(mz800_ramdisk_r);
	DECLARE_WRITE8_MEMBER(mz800_ramdisk_w);
	DECLARE_WRITE8_MEMBER(mz800_ramaddr_w);
	DECLARE_WRITE8_MEMBER(mz800_palette_w);
	DECLARE_WRITE8_MEMBER(mz800_cgram_w);
	DECLARE_DRIVER_INIT(mz800);
	DECLARE_DRIVER_INIT(mz700);
	virtual void machine_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(mz800);
	UINT32 screen_update_mz700(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mz800(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ne556_cursor_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(ne556_other_callback);
	DECLARE_WRITE_LINE_MEMBER(pit_out0_changed);
	DECLARE_WRITE_LINE_MEMBER(pit_irq_2);
	DECLARE_READ8_MEMBER(pio_port_b_r);
	DECLARE_READ8_MEMBER(pio_port_c_r);
	DECLARE_WRITE8_MEMBER(pio_port_a_w);
	DECLARE_WRITE8_MEMBER(pio_port_c_w);
	DECLARE_READ8_MEMBER(mz800_z80pio_port_a_r);
	DECLARE_WRITE8_MEMBER(mz800_z80pio_port_a_w);
};


/*----------- defined in machine/mz700.c -----------*/

extern const struct pit8253_config mz700_pit8253_config;
extern const struct pit8253_config mz800_pit8253_config;
extern const i8255_interface mz700_ppi8255_interface;
extern const z80pio_interface mz800_z80pio_config;

#endif /* MZ700_H_ */
