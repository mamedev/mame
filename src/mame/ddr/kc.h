// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*****************************************************************************
 *
 * includes/kc.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_KC_H
#define MAME_INCLUDES_KC_H

#pragma once

/* Devices */
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/timer.h"

// Components
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/ram.h"
#include "kc_keyb.h"
#include "machine/rescap.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"

// Devices
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"

// Expansions
#include "bus/kc/kc.h"
#include "bus/kc/ram.h"
#include "bus/kc/rom.h"
#include "bus/kc/d002.h"
#include "bus/kc/d004.h"

// Formats
#include "formats/kc_cas.h"

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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_z80pio(*this, "z80pio")
		, m_z80ctc(*this, "z80ctc")
		, m_ram(*this, RAM_TAG)
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_screen(*this, "screen")
		, m_expansions(*this, {"m8", "mc", "exp"})
	{ }

	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_z80pio;
	required_device<z80ctc_device> m_z80ctc;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<screen_device> m_screen;
	required_device_array<kcexp_slot_device, 3> m_expansions;

	// defined in machine/kc.cpp
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// modules read/write
	uint8_t expansion_read(offs_t offset);
	void expansion_write(offs_t offset, uint8_t data);
	uint8_t expansion_4000_r(offs_t offset);
	void expansion_4000_w(offs_t offset, uint8_t data);
	uint8_t expansion_8000_r(offs_t offset);
	void expansion_8000_w(offs_t offset, uint8_t data);
	uint8_t expansion_c000_r(offs_t offset);
	void expansion_c000_w(offs_t offset, uint8_t data);
	uint8_t expansion_e000_r(offs_t offset);
	void expansion_e000_w(offs_t offset, uint8_t data);
	uint8_t expansion_io_read(offs_t offset);
	void expansion_io_write(offs_t offset, uint8_t data);

	// bankswitch
	virtual void update_0x00000();
	virtual void update_0x04000();
	virtual void update_0x08000();
	virtual void update_0x0c000();
	virtual void update_0x0e000();

	// PIO callback
	uint8_t pio_porta_r();
	uint8_t pio_portb_r();
	DECLARE_WRITE_LINE_MEMBER( pio_ardy_cb);
	DECLARE_WRITE_LINE_MEMBER( pio_brdy_cb);
	void pio_porta_w(uint8_t data);
	void pio_portb_w(uint8_t data);

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

	// defined in video/kc.cpp
	virtual void video_start() override;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER( video_toggle_blink_state );
	void video_draw_8_pixels(bitmap_ind16 &bitmap, int x, int y, uint8_t colour_byte, uint8_t gfx_byte);

	// driver state
	uint8_t *             m_ram_base = nullptr;
	std::unique_ptr<uint8_t[]> m_video_ram{};
	int                 m_pio_data[2]{};
	int                 m_high_resolution = 0;
	uint8_t               m_ardy = 0U;
	uint8_t               m_brdy = 0U;
	int                 m_kc85_blink_state = 0;
	int                 m_k0_line = 0;
	int                 m_k1_line = 0;
	uint8_t               m_speaker_level = 0U;

	// cassette
	emu_timer *         m_cassette_timer = nullptr;
	emu_timer *         m_cassette_oneshot_timer = nullptr;
	int                 m_astb = 0;
	int                 m_cassette_in = 0;

	void kc85_palette(palette_device &palette) const;
	TIMER_CALLBACK_MEMBER(kc_cassette_oneshot_timer);
	TIMER_CALLBACK_MEMBER(kc_cassette_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(kc_scanline);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void kc85_slots(machine_config &config);

	void kc85_3(machine_config &config);
	void kc85_3_io(address_map &map);
	void kc85_3_mem(address_map &map);
};


class kc85_4_state : public kc_state
{
public:
	kc85_4_state(const machine_config &mconfig, device_type type, const char *tag)
		: kc_state(mconfig, type, tag)
	{ }

	// defined in machine/kc.cpp
	virtual void machine_reset() override;

	virtual void update_0x04000() override;
	virtual void update_0x08000() override;
	virtual void update_0x0c000() override;

	uint8_t kc85_4_86_r();
	uint8_t kc85_4_84_r();
	void kc85_4_86_w(uint8_t data);
	void kc85_4_84_w(uint8_t data);

	// defined in video/kc.cpp
	virtual void video_start() override;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	void video_control_w(int data);

	// driver state
	uint8_t               m_port_84_data = 0U;
	uint8_t               m_port_86_data = 0U;
	uint8_t *             m_display_video_ram = 0U;
	void kc85_4(machine_config &config);
	void kc85_5(machine_config &config);
	void kc85_4_io(address_map &map);
	void kc85_4_mem(address_map &map);
};

#endif // MAME_INCLUDES_KC_H
