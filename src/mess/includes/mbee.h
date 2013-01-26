/*****************************************************************************
 *
 * includes/mbee.h
 *
 ****************************************************************************/

#ifndef MBEE_H_
#define MBEE_H_

#include "machine/wd17xx.h"
#include "imagedev/snapquik.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "machine/ctronics.h"
#include "machine/mc146818.h"
#include "video/mc6845.h"
#include "sound/speaker.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/mc146818.h"
#include "sound/wave.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"


class mbee_state : public driver_device
{
public:
	mbee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "z80pio")
		, m_cass(*this, CASSETTE_TAG)
		, m_wave(*this, WAVE_TAG)
		, m_speaker(*this, SPEAKER_TAG)
		, m_printer(*this, "centronics")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_rtc(*this, "rtc")
		, m_boot(*this, "boot")
		, m_pak(*this, "pak")
		, m_telcom(*this, "telcom")
		, m_basic(*this, "basic")
		, m_bankl(*this, "bankl")
		, m_bankh(*this, "bankh")
		, m_bank1(*this, "bank1")
		, m_bank8l(*this, "bank8l")
		, m_bank8h(*this, "bank8h")
		, m_bank9(*this, "bank9")
		, m_bankfl(*this, "bankfl")
		, m_bankfh(*this, "bankfh")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_x4(*this, "X4")
		, m_io_x5(*this, "X5")
		, m_io_x6(*this, "X6")
		, m_io_x7(*this, "X7")
		, m_io_extra(*this, "EXTRA")
		, m_io_config(*this, "CONFIG")
		, m_io_x8(*this, "X8")
		, m_io_x9(*this, "X9")
		, m_io_x10(*this, "X10")
		, m_io_x11(*this, "X11")
		, m_io_x12(*this, "X12")
		, m_io_x13(*this, "X13")
		, m_io_x14(*this, "X14")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cass;
	required_device<wave_device> m_wave;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_printer;
	required_device<mc6845_device> m_crtc;
	optional_device<wd2793_device> m_fdc;
	optional_device<mc146818_device> m_rtc;
	DECLARE_WRITE8_MEMBER( mbee_04_w );
	DECLARE_WRITE8_MEMBER( mbee_06_w );
	DECLARE_READ8_MEMBER( mbee_07_r );
	DECLARE_READ8_MEMBER( mbeeic_0a_r );
	DECLARE_WRITE8_MEMBER( mbeeic_0a_w );
	DECLARE_READ8_MEMBER( mbeepc_telcom_low_r );
	DECLARE_READ8_MEMBER( mbeepc_telcom_high_r );
	DECLARE_READ8_MEMBER( mbee256_speed_low_r );
	DECLARE_READ8_MEMBER( mbee256_speed_high_r );
	DECLARE_READ8_MEMBER( mbee256_18_r );
	DECLARE_WRITE8_MEMBER( mbee64_50_w );
	DECLARE_WRITE8_MEMBER( mbee128_50_w );
	DECLARE_WRITE8_MEMBER( mbee256_50_w );
	DECLARE_READ8_MEMBER( m6545_status_r );
	DECLARE_WRITE8_MEMBER( m6545_index_w );
	DECLARE_READ8_MEMBER( m6545_data_r );
	DECLARE_WRITE8_MEMBER( m6545_data_w );
	DECLARE_READ8_MEMBER( mbee_low_r );
	DECLARE_READ8_MEMBER( mbee_high_r );
	DECLARE_READ8_MEMBER( mbeeic_high_r );
	DECLARE_WRITE8_MEMBER( mbeeic_high_w );
	DECLARE_WRITE8_MEMBER( mbee_low_w );
	DECLARE_WRITE8_MEMBER( mbee_high_w );
	DECLARE_READ8_MEMBER( mbeeic_08_r );
	DECLARE_WRITE8_MEMBER( mbeeic_08_w );
	DECLARE_READ8_MEMBER( mbee_0b_r );
	DECLARE_WRITE8_MEMBER( mbee_0b_w );
	DECLARE_READ8_MEMBER( mbeeppc_1c_r );
	DECLARE_WRITE8_MEMBER( mbeeppc_1c_w );
	DECLARE_WRITE8_MEMBER( mbee256_1c_w );
	DECLARE_READ8_MEMBER( mbeeppc_low_r );
	DECLARE_READ8_MEMBER( mbeeppc_high_r );
	DECLARE_WRITE8_MEMBER( mbeeppc_high_w );
	DECLARE_WRITE8_MEMBER( mbeeppc_low_w );
	DECLARE_WRITE8_MEMBER( pio_port_a_w );
	DECLARE_WRITE8_MEMBER( pio_port_b_w );
	DECLARE_READ8_MEMBER( pio_port_b_r );
	DECLARE_WRITE_LINE_MEMBER( pio_ardy );
	DECLARE_READ8_MEMBER(mbee_fdc_status_r);
	DECLARE_WRITE8_MEMBER(mbee_fdc_motor_w);
	DECLARE_WRITE_LINE_MEMBER(mbee_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(mbee_fdc_drq_w);
	size_t m_size;
	UINT8 m_clock_pulse;
	UINT8 m_mbee256_key_available;
	UINT8 m_fdc_intrq;
	UINT8 m_fdc_drq;
	UINT8 m_mbee256_was_pressed[15];
	UINT8 m_mbee256_q[20];
	UINT8 m_mbee256_q_pos;
	UINT8 m_framecnt;
	UINT8 *m_p_gfxram;
	UINT8 *m_p_colorram;
	UINT8 *m_p_videoram;
	UINT8 *m_p_attribram;
	UINT8 m_08;
	UINT8 m_0a;
	UINT8 m_0b;
	UINT8 m_1c;
	UINT8 m_is_premium;
	UINT8 m_sy6545_cursor[16];
	UINT8 m_sy6545_status;
	UINT8 m_speed;
	UINT8 m_flash;
	UINT16 m_cursor;
	UINT8 m_sy6545_reg[32];
	UINT8 m_sy6545_ind;
	void sy6545_cursor_configure();
	void keyboard_matrix_r(int offs);
	void mbee_video_kbd_scan(int param);
	DECLARE_DRIVER_INIT(mbeepc85);
	DECLARE_DRIVER_INIT(mbee256);
	DECLARE_DRIVER_INIT(mbee56);
	DECLARE_DRIVER_INIT(mbeett);
	DECLARE_DRIVER_INIT(mbeeppc);
	DECLARE_DRIVER_INIT(mbee);
	DECLARE_DRIVER_INIT(mbeepc);
	DECLARE_DRIVER_INIT(mbeeic);
	DECLARE_DRIVER_INIT(mbee128);
	DECLARE_DRIVER_INIT(mbee64);
	DECLARE_MACHINE_RESET(mbee);
	DECLARE_VIDEO_START(mbee);
	DECLARE_VIDEO_START(mbeeic);
	DECLARE_PALETTE_INIT(mbeeic);
	DECLARE_PALETTE_INIT(mbeepc85b);
	DECLARE_VIDEO_START(mbeeppc);
	DECLARE_PALETTE_INIT(mbeeppc);
	DECLARE_MACHINE_RESET(mbee56);
	DECLARE_MACHINE_RESET(mbee64);
	DECLARE_MACHINE_RESET(mbee128);
	DECLARE_MACHINE_RESET(mbee256);
	DECLARE_MACHINE_RESET(mbeett);
	UINT32 screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(mbee_interrupt);
	TIMER_CALLBACK_MEMBER(mbee256_kbd);
	TIMER_CALLBACK_MEMBER(mbee_rtc_irq);
	TIMER_CALLBACK_MEMBER(mbee_reset);

protected:
	required_memory_bank m_boot;
	optional_memory_bank m_pak;
	optional_memory_bank m_telcom;
	optional_memory_bank m_basic;
	optional_memory_bank m_bankl;
	optional_memory_bank m_bankh;
	optional_memory_bank m_bank1;
	optional_memory_bank m_bank8l;
	optional_memory_bank m_bank8h;
	optional_memory_bank m_bank9;
	optional_memory_bank m_bankfl;
	optional_memory_bank m_bankfh;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_x4;
	required_ioport m_io_x5;
	required_ioport m_io_x6;
	required_ioport m_io_x7;
	optional_ioport m_io_extra;
	optional_ioport m_io_config;
	optional_ioport m_io_x8;
	optional_ioport m_io_x9;
	optional_ioport m_io_x10;
	optional_ioport m_io_x11;
	optional_ioport m_io_x12;
	optional_ioport m_io_x13;
	optional_ioport m_io_x14;

	void machine_reset_common_disk();
};


/*----------- defined in machine/mbee.c -----------*/

extern const wd17xx_interface mbee_wd17xx_interface;
extern const z80pio_interface mbee_z80pio_intf;

QUICKLOAD_LOAD( mbee );


/*----------- defined in video/mbee.c -----------*/

MC6845_UPDATE_ROW( mbee_update_row );
MC6845_UPDATE_ROW( mbeeic_update_row );
MC6845_UPDATE_ROW( mbeeppc_update_row );
MC6845_ON_UPDATE_ADDR_CHANGED( mbee_update_addr );
MC6845_ON_UPDATE_ADDR_CHANGED( mbee256_update_addr );


#endif /* MBEE_H_ */
