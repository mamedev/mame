#pragma once

#ifndef __COSMICOS__
#define __COSMICOS__


#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "machine/rescap.h"
#include "sound/cdp1864.h"
#include "sound/speaker.h"
#include "video/dm9368.h"

#define CDP1802_TAG		"ic19"
#define CDP1864_TAG		"ic3"
#define DM9368_TAG		"ic10"
#define SCREEN_TAG		"screen"

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
	cosmicos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, CDP1802_TAG),
		  m_cti(*this, CDP1864_TAG),
		  m_led(*this, DM9368_TAG),
		  m_cassette(*this, CASSETTE_TAG),
		  m_speaker(*this, SPEAKER_TAG),
		  m_ram(*this, RAM_TAG)
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<dm9368_device> m_led;
	required_device<cassette_image_device> m_cassette;
	required_device<device_t> m_speaker;
	required_device<ram_device> m_ram;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( video_off_r );
	DECLARE_READ8_MEMBER( video_on_r );
	DECLARE_WRITE8_MEMBER( audio_latch_w );
	DECLARE_READ8_MEMBER( hex_keyboard_r );
	DECLARE_WRITE8_MEMBER( hex_keylatch_w );
	DECLARE_READ8_MEMBER( reset_counter_r );
	DECLARE_WRITE8_MEMBER( segment_w );
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( display_w );
	DECLARE_WRITE_LINE_MEMBER( dmaout_w );
	DECLARE_WRITE_LINE_MEMBER( efx_w );
	DECLARE_READ_LINE_MEMBER( wait_r );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef1_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_READ_LINE_MEMBER( ef4_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_READ8_MEMBER( dma_r );
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
	DECLARE_DIRECT_UPDATE_MEMBER(cosmicos_direct_update_handler);

	void set_cdp1802_mode(int mode);
	void clear_input_data();
	void set_ram_mode();


	/* CPU state */
	int m_wait;
	int m_clear;
	int m_sc1;

	/* memory state */
	UINT8 m_data;
	int m_boot;
	int m_ram_protect;
	int m_ram_disable;

	/* keyboard state */
	UINT8 m_keylatch;

	/* display state */
	UINT8 m_segment;
	int m_digit;
	int m_counter;
	int m_q;
	int m_dmaout;
	int m_efx;
	int m_video_on;
	DECLARE_DRIVER_INIT(cosmicos);
};

#endif
