// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_COSMICOS_H
#define MAME_INCLUDES_COSMICOS_H

#pragma once

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/cdp1864.h"
#include "sound/spkrdev.h"
#include "video/dm9368.h"

#define CDP1802_TAG     "ic19"
#define CDP1864_TAG     "ic3"
#define DM9368_TAG      "ic10"
#define SCREEN_TAG      "screen"

enum
{
	LED_RUN = 0,
	LED_LOAD,
	LED_PAUSE,
	LED_RESET,
	LED_D7,
	LED_D6,
	LED_D5,
	LED_D4,
	LED_D3,
	LED_D2,
	LED_D1,
	LED_D0,
	LED_Q,
	LED_CASSETTE
};

class cosmicos_state : public driver_device
{
public:
	cosmicos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_digit(0),
		m_maincpu(*this, CDP1802_TAG),
		m_cti(*this, CDP1864_TAG),
		m_led(*this, DM9368_TAG),
		m_cassette(*this, "cassette"),
		m_speaker(*this, "speaker"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, CDP1802_TAG),
		m_key_row(*this, {"Y1", "Y2", "Y3", "Y4"}),
		m_io_data(*this, "DATA"),
		m_special(*this, "SPECIAL"),
		m_buttons(*this, "BUTTONS"),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER( data );
	DECLARE_INPUT_CHANGED_MEMBER( enter );
	DECLARE_INPUT_CHANGED_MEMBER( single_step );
	DECLARE_INPUT_CHANGED_MEMBER( run );
	DECLARE_INPUT_CHANGED_MEMBER( load );
	DECLARE_INPUT_CHANGED_MEMBER( cosmicos_pause );
	DECLARE_INPUT_CHANGED_MEMBER( reset );
	DECLARE_INPUT_CHANGED_MEMBER( clear_data );
	DECLARE_INPUT_CHANGED_MEMBER( memory_protect );
	DECLARE_INPUT_CHANGED_MEMBER( memory_disable );

	void init_cosmicos();
	void cosmicos(machine_config &config);

private:
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t video_off_r();
	uint8_t video_on_r();
	void audio_latch_w(uint8_t data);
	uint8_t hex_keyboard_r();
	void hex_keylatch_w(uint8_t data);
	uint8_t reset_counter_r();
	void segment_w(uint8_t data);
	uint8_t data_r();
	void display_w(uint8_t data);
	uint8_t dma_r();
	void sc_w(uint8_t data);
	void set_cdp1802_mode(int mode);
	void clear_input_data();
	DECLARE_WRITE_LINE_MEMBER( dmaout_w );
	DECLARE_WRITE_LINE_MEMBER( efx_w );
	DECLARE_READ_LINE_MEMBER( wait_r );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef1_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(digit_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(int_tick);
	void cosmicos_io(address_map &map);
	void cosmicos_mem(address_map &map);

	/* CPU state */
	int m_wait = 0;
	int m_clear = 0;
	int m_sc1 = 0;

	/* memory state */
	uint8_t m_data = 0U;
	int m_boot = 0;
	int m_ram_protect = 0;
	int m_ram_disable = 0;

	/* keyboard state */
	uint8_t m_keylatch = 0U;

	/* display state */
	uint8_t m_segment = 0U;
	int m_digit = 0;
	int m_counter = 0;
	int m_q = 0;
	int m_dmaout = 0;
	int m_efx = 0;
	int m_video_on = 0;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<dm9368_device> m_led;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_ioport_array<4> m_key_row;
	required_ioport m_io_data;
	required_ioport m_special;
	required_ioport m_buttons;
	output_finder<10> m_digits;
	output_finder<14> m_leds;
};

#endif // MAME_INCLUDES_COSMICOS_H
