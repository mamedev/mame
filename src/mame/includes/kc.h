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
	uint8_t expansion_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void expansion_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t expansion_4000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void expansion_4000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t expansion_8000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void expansion_8000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t expansion_c000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void expansion_c000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t expansion_e000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void expansion_e000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t expansion_io_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void expansion_io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// bankswitch
	virtual void update_0x00000();
	virtual void update_0x04000();
	virtual void update_0x08000();
	virtual void update_0x0c000();
	virtual void update_0x0e000();

	// PIO callback
	uint8_t pio_porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t pio_portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio_ardy_cb(int state);
	void pio_brdy_cb(int state);
	void pio_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pio_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// CTC callback
	void ctc_zc0_callback(int state);
	void ctc_zc1_callback(int state);

	// keyboard
	void keyboard_cb(int state);

	// cassette
	void update_cassette(int state);
	void cassette_set_motor(int motor_state);

	// speaker
	void speaker_update();

	// defined in video/kc.c
	virtual void video_start() override;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_toggle_blink_state(int state);
	void video_draw_8_pixels(bitmap_ind16 &bitmap, int x, int y, uint8_t colour_byte, uint8_t gfx_byte);

	// driver state
	uint8_t *             m_ram_base;
	uint8_t *             m_video_ram;
	int                 m_pio_data[2];
	int                 m_high_resolution;
	uint8_t               m_ardy;
	uint8_t               m_brdy;
	int                 m_kc85_blink_state;
	int                 m_k0_line;
	int                 m_k1_line;
	uint8_t               m_speaker_level;

	// cassette
	emu_timer *         m_cassette_timer;
	emu_timer *         m_cassette_oneshot_timer;
	int                 m_astb;
	int                 m_cassette_in;

	kcexp_slot_device * m_expansions[3];
	void palette_init_kc85(palette_device &palette);
	void kc_cassette_oneshot_timer(void *ptr, int32_t param);
	void kc_cassette_timer_callback(void *ptr, int32_t param);
	void kc_scanline(timer_device &timer, void *ptr, int32_t param);

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

	uint8_t kc85_4_86_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t kc85_4_84_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kc85_4_86_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kc85_4_84_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// defined in video/kc.c
	virtual void video_start() override;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void video_control_w(int data);

	// driver state
	uint8_t               m_port_84_data;
	uint8_t               m_port_86_data;
	uint8_t *             m_display_video_ram;
};

#endif /* KC_H_ */
