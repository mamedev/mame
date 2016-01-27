// license:GPL-2.0+
// copyright-holders:Lee Hammerton, Dirk Best
/*****************************************************************************
 *
 * includes/coupe.h
 *
 * SAM Coupe
 *
 * Driver by Lee Hammerton
 *
 ****************************************************************************/

#ifndef SAMCOUPE_H_
#define SAMCOUPE_H_

#include "machine/wd_fdc.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "machine/ram.h"
#include "machine/msm6242.h"

/* screen dimensions */
#define SAM_BLOCK           8

#define SAM_TOTAL_WIDTH     SAM_BLOCK*96
#define SAM_TOTAL_HEIGHT    312
#define SAM_SCREEN_WIDTH    SAM_BLOCK*64
#define SAM_SCREEN_HEIGHT   192
#define SAM_BORDER_LEFT     SAM_BLOCK*4
#define SAM_BORDER_RIGHT    SAM_BLOCK*4
#define SAM_BORDER_TOP      37
#define SAM_BORDER_BOTTOM   46

/* interrupt sources */
#define SAM_LINE_INT     0x01
#define SAM_MOUSE_INT    0x02
#define SAM_MIDIIN_INT   0x04
#define SAM_FRAME_INT    0x08
#define SAM_MIDIOUT_INT  0x10


class samcoupe_state :  public driver_device
{
public:
	enum
	{
		TIMER_IRQ_OFF,
		TIMER_MOUSE_RESET,
		TIMER_VIDEO_UPDATE
	};

	samcoupe_state(const machine_config &mconfig, device_type type, const char *tag)
			: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_speaker(*this, "speaker"),
			m_cassette(*this, "cassette"),
			m_lpt1(*this, "lpt1"),
			m_lpt2(*this, "lpt2"),
			m_ram(*this, RAM_TAG),
			m_rtc(*this, "sambus_clock"),
			m_fdc(*this, "wd1772"),
			m_wd1772_0(*this, "wd1772:0"),
			m_wd1772_1(*this, "wd1772:1"),
			m_region_maincpu(*this, "maincpu"),
			m_keyboard_row_fe(*this, "keyboard_row_fe"),
			m_keyboard_row_fd(*this, "keyboard_row_fd"),
			m_keyboard_row_fb(*this, "keyboard_row_fb"),
			m_keyboard_row_f7(*this, "keyboard_row_f7"),
			m_keyboard_row_ef(*this, "keyboard_row_ef"),
			m_keyboard_row_df(*this, "keyboard_row_df"),
			m_keyboard_row_bf(*this, "keyboard_row_bf"),
			m_keyboard_row_7f(*this, "keyboard_row_7f"),
			m_keyboard_row_ff(*this, "keyboard_row_ff"),
			m_mouse_buttons(*this, "mouse_buttons"),
			m_io_mouse_x(*this, "mouse_x"),
			m_io_mouse_y(*this, "mouse_y"),
			m_config(*this, "config")
	{
		sam_bank_read_ptr[0] = nullptr;
		sam_bank_write_ptr[0] = nullptr;
		sam_bank_read_ptr[1] = nullptr;
		sam_bank_write_ptr[1] = nullptr;
		sam_bank_read_ptr[2] = nullptr;
		sam_bank_write_ptr[2] = nullptr;
		sam_bank_read_ptr[3] = nullptr;
		sam_bank_write_ptr[3] = nullptr;
	}

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	emu_timer *m_video_update_timer;

	UINT8 m_lmpr, m_hmpr, m_vmpr; /* memory pages */
	UINT8 m_lext, m_hext;       /* extended memory page */
	UINT8 m_border;           /* border */
	UINT8 m_clut[16];         /* color lookup table, 16 entries */
	UINT8 m_line_int;         /* line interrupt */
	UINT8 m_status;           /* status register */

	/* attribute */
	UINT8 m_attribute;

	/* mouse */
	int m_mouse_index;
	emu_timer *m_mouse_reset;
	UINT8 m_mouse_data[9];
	int m_mouse_x, m_mouse_y;
	UINT8 *m_videoram;
	DECLARE_WRITE8_MEMBER(samcoupe_ext_mem_w);
	DECLARE_READ8_MEMBER(samcoupe_disk_r);
	DECLARE_WRITE8_MEMBER(samcoupe_disk_w);
	DECLARE_READ8_MEMBER(samcoupe_pen_r);
	DECLARE_WRITE8_MEMBER(samcoupe_clut_w);
	DECLARE_READ8_MEMBER(samcoupe_status_r);
	DECLARE_WRITE8_MEMBER(samcoupe_line_int_w);
	DECLARE_READ8_MEMBER(samcoupe_lmpr_r);
	DECLARE_WRITE8_MEMBER(samcoupe_lmpr_w);
	DECLARE_READ8_MEMBER(samcoupe_hmpr_r);
	DECLARE_WRITE8_MEMBER(samcoupe_hmpr_w);
	DECLARE_READ8_MEMBER(samcoupe_vmpr_r);
	DECLARE_WRITE8_MEMBER(samcoupe_vmpr_w);
	DECLARE_READ8_MEMBER(samcoupe_midi_r);
	DECLARE_WRITE8_MEMBER(samcoupe_midi_w);
	DECLARE_READ8_MEMBER(samcoupe_keyboard_r);
	DECLARE_WRITE8_MEMBER(samcoupe_border_w);
	DECLARE_READ8_MEMBER(samcoupe_attributes_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(samcoupe);
	INTERRUPT_GEN_MEMBER(samcoupe_frame_interrupt);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(samcoupe_mouse_reset);
	TIMER_CALLBACK_MEMBER(sam_video_update_callback);
	DECLARE_READ8_MEMBER(samcoupe_lpt1_busy_r);
	DECLARE_WRITE8_MEMBER(samcoupe_lpt1_strobe_w);
	DECLARE_READ8_MEMBER(samcoupe_lpt2_busy_r);
	DECLARE_WRITE8_MEMBER(samcoupe_lpt2_strobe_w);
	DECLARE_READ8_MEMBER(samcoupe_rtc_r);
	DECLARE_WRITE8_MEMBER(samcoupe_rtc_w);

	DECLARE_READ8_MEMBER(sam_bank1_r);
	DECLARE_WRITE8_MEMBER(sam_bank1_w);
	DECLARE_READ8_MEMBER(sam_bank2_r);
	DECLARE_WRITE8_MEMBER(sam_bank2_w);
	DECLARE_READ8_MEMBER(sam_bank3_r);
	DECLARE_WRITE8_MEMBER(sam_bank3_w);
	DECLARE_READ8_MEMBER(sam_bank4_r);
	DECLARE_WRITE8_MEMBER(sam_bank4_w);

	UINT8* sam_bank_read_ptr[4];
	UINT8* sam_bank_write_ptr[4];
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_lpt1;
	required_device<centronics_device> m_lpt2;
	required_device<ram_device> m_ram;
	required_device<msm6242_device> m_rtc;
	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_wd1772_0;
	required_device<floppy_connector> m_wd1772_1;
	required_memory_region m_region_maincpu;
	required_ioport m_keyboard_row_fe;
	required_ioport m_keyboard_row_fd;
	required_ioport m_keyboard_row_fb;
	required_ioport m_keyboard_row_f7;
	required_ioport m_keyboard_row_ef;
	required_ioport m_keyboard_row_df;
	required_ioport m_keyboard_row_bf;
	required_ioport m_keyboard_row_7f;
	required_ioport m_keyboard_row_ff;
	required_ioport m_mouse_buttons;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;
	required_ioport m_config;

	void draw_mode4_line(int y, int hpos);
	void draw_mode3_line(int y, int hpos);
	void draw_mode12_block(bitmap_ind16 &bitmap, int vpos, int hpos, UINT8 mask);
	void draw_mode2_line(int y, int hpos);
	void draw_mode1_line(int y, int hpos);
	void samcoupe_update_bank(address_space &space, int bank_num, UINT8 *memory, int is_readonly);
	void samcoupe_install_ext_mem(address_space &space);
	void samcoupe_update_memory(address_space &space);
	UINT8 samcoupe_mouse_r();
	void samcoupe_irq(UINT8 src);

	DECLARE_WRITE_LINE_MEMBER(write_lpt1_busy);
	DECLARE_WRITE_LINE_MEMBER(write_lpt2_busy);

	int m_lpt1_busy;
	int m_lpt2_busy;
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif /* SAMCOUPE_H_ */
