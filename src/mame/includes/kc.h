// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*****************************************************************************
 *
 * includes/kc.h
 *
 ****************************************************************************/

#ifndef KC_H_
#define KC_H_

/* Devices */
#include "imagedev/cassette.h"
#include "machine/ram.h"

// Components
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/ram.h"
#include "machine/kc_keyb.h"
#include "machine/rescap.h"
#include "cpu/z80/z80daisy.h"
#include "sound/speaker.h"
#include "sound/wave.h"

// Devices
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"

// Formats
#include "formats/kc_cas.h"

// Expansions
#include "bus/kc/kc.h"
#include "bus/kc/ram.h"
#include "bus/kc/rom.h"
#include "bus/kc/d002.h"
#include "bus/kc/d004.h"

// from service manual
#define KC85_3_CLOCK 1751938
#define KC85_4_CLOCK 1773447

#define KC85_4_SCREEN_PIXEL_RAM_SIZE 0x04000
#define KC85_4_SCREEN_COLOUR_RAM_SIZE 0x04000

#define KC85_PALETTE_SIZE 24
#define KC85_SCREEN_WIDTH 320
#define KC85_SCREEN_HEIGHT 256

// cassette input polling frequency
#define KC_CASSETTE_TIMER_FREQUENCY attotime::from_hz(44100)


class kc_state : public driver_device
{
public:
	kc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_z80pio(*this, "z80pio"),
			m_z80ctc(*this, "z80ctc"),
			m_ram(*this, RAM_TAG),
			m_speaker(*this, "speaker"),
			m_cassette(*this, "cassette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_z80pio;
	required_device<z80ctc_device> m_z80ctc;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;

	// defined in machine/kc.c
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// modules read/write
	DECLARE_READ8_MEMBER ( expansion_read );
	DECLARE_WRITE8_MEMBER( expansion_write );
	DECLARE_READ8_MEMBER ( expansion_4000_r );
	DECLARE_WRITE8_MEMBER( expansion_4000_w );
	DECLARE_READ8_MEMBER ( expansion_8000_r );
	DECLARE_WRITE8_MEMBER( expansion_8000_w );
	DECLARE_READ8_MEMBER ( expansion_c000_r );
	DECLARE_WRITE8_MEMBER( expansion_c000_w );
	DECLARE_READ8_MEMBER ( expansion_e000_r );
	DECLARE_WRITE8_MEMBER( expansion_e000_w );
	DECLARE_READ8_MEMBER ( expansion_io_read );
	DECLARE_WRITE8_MEMBER( expansion_io_write );

	// bankswitch
	virtual void update_0x00000();
	virtual void update_0x04000();
	virtual void update_0x08000();
	virtual void update_0x0c000();
	virtual void update_0x0e000();

	// PIO callback
	DECLARE_READ8_MEMBER( pio_porta_r );
	DECLARE_READ8_MEMBER( pio_portb_r );
	DECLARE_WRITE_LINE_MEMBER( pio_ardy_cb);
	DECLARE_WRITE_LINE_MEMBER( pio_brdy_cb);
	DECLARE_WRITE8_MEMBER( pio_porta_w );
	DECLARE_WRITE8_MEMBER( pio_portb_w );

	// CTC callback
	DECLARE_WRITE_LINE_MEMBER( ctc_zc0_callback );
	DECLARE_WRITE_LINE_MEMBER( ctc_zc1_callback );

	// keyboard
	DECLARE_WRITE_LINE_MEMBER( keyboard_cb );

	// cassette
	void update_cassette(int state);
	void cassette_set_motor(int motor_state);

	// speaker
	void speaker_update();

	// defined in video/kc.c
	virtual void video_start() override;
	virtual UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER( video_toggle_blink_state );
	void video_draw_8_pixels(bitmap_ind16 &bitmap, int x, int y, UINT8 colour_byte, UINT8 gfx_byte);

	// driver state
	UINT8 *             m_ram_base;
	UINT8 *             m_video_ram;
	int                 m_pio_data[2];
	int                 m_high_resolution;
	UINT8               m_ardy;
	UINT8               m_brdy;
	int                 m_kc85_blink_state;
	int                 m_k0_line;
	int                 m_k1_line;
	UINT8               m_speaker_level;

	// cassette
	emu_timer *         m_cassette_timer;
	emu_timer *         m_cassette_oneshot_timer;
	int                 m_astb;
	int                 m_cassette_in;

	kcexp_slot_device * m_expansions[3];
	DECLARE_PALETTE_INIT(kc85);
	TIMER_CALLBACK_MEMBER(kc_cassette_oneshot_timer);
	TIMER_CALLBACK_MEMBER(kc_cassette_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(kc_scanline);

	DECLARE_QUICKLOAD_LOAD_MEMBER( kc );
};


class kc85_4_state : public kc_state
{
public:
	kc85_4_state(const machine_config &mconfig, device_type type, const char *tag)
		: kc_state(mconfig, type, tag)
		{ }

	// defined in machine/kc.c
	virtual void machine_reset() override;

	virtual void update_0x04000() override;
	virtual void update_0x08000() override;
	virtual void update_0x0c000() override;

	DECLARE_READ8_MEMBER( kc85_4_86_r );
	DECLARE_READ8_MEMBER( kc85_4_84_r );
	DECLARE_WRITE8_MEMBER( kc85_4_86_w );
	DECLARE_WRITE8_MEMBER( kc85_4_84_w );

	// defined in video/kc.c
	virtual void video_start() override;
	virtual UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void video_control_w(int data);

	// driver state
	UINT8               m_port_84_data;
	UINT8               m_port_86_data;
	UINT8 *             m_display_video_ram;
};

#endif /* KC_H_ */
